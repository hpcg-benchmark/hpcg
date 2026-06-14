
//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*!
 @file ComputeSYMGS.cpp

 HPCG routine
 */
#ifdef USE_CUDA
#include "Cuda.hpp"
#endif
#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#endif
#include "ComputeSPMV.hpp"
#include "ComputeSYMGS.hpp"
#include "CpuKernels.hpp"
#include "CudaKernels.hpp"

/*!
  Routine to compute one step of symmetric Gauss-Seidel:

  Assumption about the structure of matrix A:
  - Each row 'i' of the matrix has nonzero diagonal value whose address is matrixDiagonal[i]
  - Entries in row 'i' are ordered such that:
       - lower triangular terms are stored before the diagonal element.
       - upper triangular terms are stored after the diagonal element.
       - No other assumptions are made about entry ordering.

  Symmetric Gauss-Seidel notes:
  - We use the input vector x as the RHS and start with an initial guess for y of all zeros.
  - We perform one forward sweep.  Since y is initially zero we can ignore the upper triangular terms of A.
  - We then perform one back sweep.
       - For simplicity we include the diagonal contribution in the for-j loop, then correct the sum after

  @param[in] A the known system matrix
  @param[in] r the input vector
  @param[inout] x On entry, x should contain relevant values, on exit x contains the result of one symmetric GS sweep
  with r as the RHS.

  @return returns 0 upon success and non-zero otherwise

  @warning Early versions of this kernel (Version 1.1 and earlier) had the r and x arguments in reverse order, and out
  of sync with other kernels.

  @see ComputeSYMGS_ref
*/

#ifdef USE_CUDA
int ComputeSYMGS_Gpu(const SparseMatrix& A, const Vector& r, Vector& x, bool step)
{
    double* tmp_d;
    if (step == 1 && A.mgData != 0)
    {
        tmp_d = (*A.mgData->Axf).values_d;
    }
    else
    {
        tmp_d = A.tempBuffer;
    }
    const local_int_t nrow = A.localNumberOfRows;
    double alpha = 1.0;
    cusparseFillMode_t fillmode_l = CUSPARSE_FILL_MODE_LOWER;
    cusparseFillMode_t fillmode_u = CUSPARSE_FILL_MODE_UPPER;

    if (step == 1)
    {
        // TRSV(D+L, r, t)
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecX, r.values_d));
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecY, tmp_d));
        CHECK_CUSPARSE(cusparseSpMatSetAttribute(
            A.cusparseOpt.matA, CUSPARSE_SPMAT_FILL_MODE, &(fillmode_l), sizeof(fillmode_l)));
        CHECK_CUSPARSE(cusparseSpSV_solve(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha, A.cusparseOpt.matA,
            A.cusparseOpt.vecX, A.cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPSV_ALG_DEFAULT, A.cusparseOpt.spsvDescrL));

        // SPMV(D, t, t)
        SpmvDiagCuda(nrow, tmp_d, A.diagonal);

        // TRSV(D+U, t, x)
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecX, tmp_d));
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecY, x.values_d));
        CHECK_CUSPARSE(cusparseSpMatSetAttribute(
            A.cusparseOpt.matA, CUSPARSE_SPMAT_FILL_MODE, &(fillmode_u), sizeof(fillmode_u)));
        CHECK_CUSPARSE(cusparseSpSV_solve(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha, A.cusparseOpt.matA,
            A.cusparseOpt.vecX, A.cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPSV_ALG_DEFAULT, A.cusparseOpt.spsvDescrU));

        if (A.mgData != 0)
        {
#ifndef HPCG_NO_MPI
            CHECK_CUDART(cudaStreamSynchronize(stream));
            PackSendBufferCuda(A, x, false, copy_stream);
#endif

            // SPMV(L, x, t): t = t + L * x
            double alpha = 1.0;
            CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecX, x.values_d));
            CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecY, (*A.mgData->Axf).values_d));
            CHECK_CUSPARSE(cusparseSpMV(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha, A.cusparseOpt.matL,
                A.cusparseOpt.vecX, &alpha, A.cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPMV_ALG_DEFAULT, A.bufferMvA));

#ifndef HPCG_NO_MPI
            if (A.totalToBeSent > 0)
            {
                ExchangeHaloCuda(A, x, copy_stream);
                double one = 1.0, zero = 0.0;
                ExtSpMVCuda((SparseMatrix&) A, one, x.values_d + A.localNumberOfRows, (*A.mgData->Axf).values_d);
            }
#endif
        }
    }
    else
    { // step == 0
#ifndef HPCG_NO_MPI
        CHECK_CUDART(cudaStreamSynchronize(stream));
        PackSendBufferCuda(A, x, false, copy_stream);
#endif

        // SPMV(U, x, t): t = U * x
        double alpha = 1.0, beta = 0.0;
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecX, x.values_d));
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecY, (*A.mgData->Axf).values_d));
        CHECK_CUSPARSE(cusparseSpMV(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha, A.cusparseOpt.matU,
            A.cusparseOpt.vecX, &beta, A.cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPMV_ALG_DEFAULT, A.bufferMvA));

        // tmp = rv - t
        AxpbyCuda(nrow, r.values_d, (*A.mgData->Axf).values_d, tmp_d);

#ifndef HPCG_NO_MPI
        if (A.totalToBeSent > 0)
        {
            // MPI_Ibarrier --> will help improve MPI_Allreduce in dot product
            ExchangeHaloCuda(A, x, copy_stream, A.level == 0 ? 1 /*call MPI_Ibarrier*/ : 0);
            double mone = -1.0, zero = 0.0;
            ExtSpMVCuda((SparseMatrix&) A, mone, x.values_d + A.localNumberOfRows, tmp_d);
        }
#endif

        // TRSV(D+L, r-t, x)
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecX, tmp_d));
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecY, x.values_d));
        CHECK_CUSPARSE(cusparseSpMatSetAttribute(
            A.cusparseOpt.matA, CUSPARSE_SPMAT_FILL_MODE, &(fillmode_l), sizeof(fillmode_l)));
        CHECK_CUSPARSE(cusparseSpSV_solve(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha, A.cusparseOpt.matA,
            A.cusparseOpt.vecX, A.cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPSV_ALG_DEFAULT, A.cusparseOpt.spsvDescrL));

        // SPMV(D, x, t) t += D*x
        SpFmaCuda(nrow, x.values_d, A.diagonal, (*A.mgData->Axf).values_d);

        // TRSV(D+U, x, x)
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecX, (*A.mgData->Axf).values_d));
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecY, x.values_d));
        CHECK_CUSPARSE(cusparseSpMatSetAttribute(
            A.cusparseOpt.matA, CUSPARSE_SPMAT_FILL_MODE, &(fillmode_u), sizeof(fillmode_u)));
        CHECK_CUSPARSE(cusparseSpSV_solve(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha, A.cusparseOpt.matA,
            A.cusparseOpt.vecX, A.cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPSV_ALG_DEFAULT, A.cusparseOpt.spsvDescrU));
    }
    return 0;
}
#endif

#ifdef USE_GRACE
int ComputeSYMGS_Cpu(const SparseMatrix& A, const Vector& r, Vector& x, bool step)
{
    local_int_t nrow = A.localNumberOfRows;
    double* temp;
    if (step == 1 && A.mgData != 0)
    {
        temp = (*A.mgData->Axf).values;
    }
    else
    {
        temp = A.tempBuffer;
    }
    double* xv = x.values;
    double* rv = r.values;
    double one = 1.0, zero = 0.0;
    nvpl_sparse_fill_mode_t fillmode_l = NVPL_SPARSE_FILL_MODE_LOWER;
    nvpl_sparse_fill_mode_t fillmode_u = NVPL_SPARSE_FILL_MODE_UPPER;

    if (step == 1)
    {
        // TRSV(L, r, x)
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecX, r.values);
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecY, xv);
        nvpl_sparse_sp_mat_set_attribute(
            A.nvplSparseOpt.matL, NVPL_SPARSE_SPMAT_FILL_MODE, &(fillmode_l), sizeof(fillmode_l));
        nvpl_sparse_spsv_solve(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &one, A.nvplSparseOpt.matL,
            A.nvplSparseOpt.vecX, A.nvplSparseOpt.vecY, NVPL_SPARSE_R_64F, NVPL_SPARSE_SPSV_ALG_DEFAULT,
            A.nvplSparseOpt.spsvDescrL);

        // SPMV(D, x, t) t = D*x
        SpmvDiagCpu(nrow, A.diagonal, xv, temp);

        // TRSV(U, x, x)
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecX, temp);
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecY, xv);
        nvpl_sparse_sp_mat_set_attribute(
            A.nvplSparseOpt.matU, NVPL_SPARSE_SPMAT_FILL_MODE, &(fillmode_u), sizeof(fillmode_u));
        nvpl_sparse_spsv_solve(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &one, A.nvplSparseOpt.matU,
            A.nvplSparseOpt.vecX, A.nvplSparseOpt.vecY, NVPL_SPARSE_R_64F, NVPL_SPARSE_SPSV_ALG_DEFAULT,
            A.nvplSparseOpt.spsvDescrU);

        if (A.mgData != 0)
        {
            // SPMV(L, x, t): t += L*x
            nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecX, xv);
            nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecY, temp);
            nvpl_sparse_spmv(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &one, A.nvplSparseOpt.matL,
                A.nvplSparseOpt.vecX, &one, A.nvplSparseOpt.vecY, A.nvplSparseOpt.vecY, NVPL_SPARSE_R_64F,
                NVPL_SPARSE_SPMV_ALG_DEFAULT, A.nvplSparseOpt.spmvLDescr);

#ifndef HPCG_NO_MPI
            ExchangeHaloCpu(A, x);
            if (A.totalToBeSent > 0)
            {
                ExtSpMVCpu(A, nrow, 1.0, xv, temp);
            }
#endif
        }
    }
    else if (step == 0)
    {
        // SPMV(U, x, t) t = U*x
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecX, xv);
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecY, (*A.mgData->Axf).values);

        nvpl_sparse_spmv(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &one, A.nvplSparseOpt.matU,
            A.nvplSparseOpt.vecX, &zero, A.nvplSparseOpt.vecY, A.nvplSparseOpt.vecY, NVPL_SPARSE_R_64F,
            NVPL_SPARSE_SPMV_ALG_DEFAULT, A.nvplSparseOpt.spmvUDescr);

        // axpy: t = r-t
        AxpbyCpu(nrow, rv, (*A.mgData->Axf).values, temp);

#ifndef HPCG_NO_MPI
        // MPI_Ibarrier --> will help improve MPI_Allreduce in dot product
        ExchangeHaloCpu(A, x, A.level == 0 ? 1 /*call MPI_Ibarrier*/ : 0);
        if (A.totalToBeSent > 0)
        {
            ExtSpMVCpu(A, nrow, -1.0, xv, temp);
        }
#endif

        // TRSV(L, r-t, x)
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecX, temp);
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecY, xv);
        nvpl_sparse_sp_mat_set_attribute(
            A.nvplSparseOpt.matL, NVPL_SPARSE_SPMAT_FILL_MODE, &(fillmode_l), sizeof(fillmode_l));
        nvpl_sparse_spsv_solve(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &one, A.nvplSparseOpt.matL,
            A.nvplSparseOpt.vecX, A.nvplSparseOpt.vecY, NVPL_SPARSE_R_64F, NVPL_SPARSE_SPSV_ALG_DEFAULT,
            A.nvplSparseOpt.spsvDescrL);

        // SPMV(D, x, t) t += D*x
        SpFmaCpu(nrow, A.diagonal, xv, (*A.mgData->Axf).values);

        // TRSV(U, x, x)
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecX, (*A.mgData->Axf).values);
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecY, xv);
        nvpl_sparse_sp_mat_set_attribute(
            A.nvplSparseOpt.matU, NVPL_SPARSE_SPMAT_FILL_MODE, &(fillmode_u), sizeof(fillmode_u));
        nvpl_sparse_spsv_solve(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &one, A.nvplSparseOpt.matU,
            A.nvplSparseOpt.vecX, A.nvplSparseOpt.vecY, NVPL_SPARSE_R_64F, NVPL_SPARSE_SPSV_ALG_DEFAULT,
            A.nvplSparseOpt.spsvDescrU);
    }

    return 0;
}
#endif // USE_GRACE

int ComputeSYMGS(const SparseMatrix& A, const Vector& r, Vector& x, bool step)
{
    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
        ComputeSYMGS_Gpu(A, r, x, step);
#endif
    }
    else
    {
#ifdef USE_GRACE
        ComputeSYMGS_Cpu(A, r, x, step);
#endif
    }

    return 0;
}
