
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
 @file OptimizeProblem.cpp

 HPCG routine
 */

#include "OptimizeProblem.hpp"
#include "CpuKernels.hpp"
#include "CudaKernels.hpp"
#include "Cuda.hpp"
#include "WriteProblem.hpp"
#include "mytimer.hpp"

extern bool Use_Hpcg_Mem_Reduction; /*USE HPCG aggresive memory reduction*/

/*!
  Optimizes the data structures used for CG iteration to increase the
  performance of the benchmark version of the preconditioned CG algorithm.

  @param[inout] A      The known system matrix, also contains the MG hierarchy in attributes Ac and mgData.
  @param[inout] data   The data structure with all necessary CG vectors preallocated
  @param[inout] b      The known right hand side vector
  @param[inout] x      The solution vector to be computed in future CG iteration
  @param[inout] xexact The exact solution vector

  @return returns 0 upon success and non-zero otherwise

  @see GenerateGeometry
  @see GenerateProblem
*/

#ifdef USE_CUDA
size_t OptimizeProblemGpu(SparseMatrix& A_in, CGData& data, Vector& b, Vector& x, Vector& xexact)
{
    // This function can be used to completely transform any part of the data structures.
    // Right now it does nothing, so compiling with a check for unused variables results in complaints
    SparseMatrix* A = &A_in;
    local_int_t numberOfMgLevels = 4;
    local_int_t slice_size = A->slice_size;
    for (int level = 0; level < numberOfMgLevels; ++level)
    {
        const local_int_t nrow = A->localNumberOfRows;
        int totalColors = 8;

        // Let's deal with perm and iperm
        SetVectorAscCuda(A->ref2opt, nrow);
        SetVectorAscCuda(A->opt2ref, nrow);

        // Let us color the matrix
        int num_colors = 0;
        ColorMatrixCuda(NULL, A->gpuAux.columns, A->gpuAux.nnzPerRow, A->localNumberOfRows, A->gpuAux.color,
            &(num_colors), A->gpuAux.colorCountCpu, 8, A->ref2opt, A->opt2ref, A->geom->rank, A->geom->nx, NULL);
        A->totalColors = totalColors;
        PermElemToSendCuda(A->totalToBeSent, A->gpuAux.elementsToSend, A->ref2opt);

        // Create (S)ELL
        local_int_t TranslateIndex = slice_size * HPCG_MAX_ROW_LEN;
        local_int_t* translated_ell_col_index = A->sellAPermColumns + TranslateIndex;
        double* translated_ell_values = A->sellAPermValues + TranslateIndex;

        EllPermColumnsValuesCuda(nrow, A->gpuAux.nnzPerRow, A->gpuAux.columns, A->gpuAux.values,
            A->gpuAux.csrAPermOffsets, translated_ell_col_index, translated_ell_values, A->opt2ref, A->ref2opt,
            A->gpuAux.sellADiagonalIdx, A->gpuAux.csrLPermOffsets, A->gpuAux.csrUPermOffsets, false);

        // Coloumn mojor blocked/sliced ellpack
        TransposeCuda(nrow, slice_size, A->sellAPermColumns, A->sellAPermValues);

        // Per block max row len
        local_int_t num_slices = (nrow + slice_size - 1) / slice_size;
        EllMaxRowLenPerBlockCuda(nrow, slice_size, A->gpuAux.csrLPermOffsets, A->gpuAux.csrUPermOffsets,
            A->sellLSliceMrl, A->sellUSliceMrl);

        // Find prefix sum for sliced ell
        PrefixsumCuda(num_slices, A->sellLSliceMrl);
        MultiplyBySliceSizeCUDA(num_slices, slice_size, A->sellLSliceMrl + 1);

        PrefixsumCuda(num_slices, A->sellUSliceMrl);
        MultiplyBySliceSizeCUDA(num_slices, slice_size, A->sellUSliceMrl + 1);

        // Set the general matrix slice_offsets
        CreateAMatrixSliceOffsetsCuda(num_slices + 1, A->slice_size, A->sellASliceMrl);

        // Lower Upper ELL variant parts
        CreateSellLUColumnsValuesCuda(nrow, slice_size, A->sellAPermColumns, A->sellAPermValues, A->sellLSliceMrl,
            A->sellLPermColumns, A->sellLPermValues, A->sellUSliceMrl, A->sellUPermColumns, A->sellUPermValues, level);

        local_int_t sell_slices = (nrow + slice_size - 1) / slice_size;
        const local_int_t half_nnz = (A->localNumberOfNonzeros - nrow - A->extNnz) / 2;

        local_int_t sell_l_nnz = 0;
        CHECK_CUDART(cudaMemcpyAsync(
            &sell_l_nnz, &(A->sellLSliceMrl[sell_slices]), sizeof(local_int_t), cudaMemcpyDeviceToHost, stream));

        local_int_t sell_u_nnz = 0;
        CHECK_CUDART(cudaMemcpyAsync(
            &sell_u_nnz, &(A->sellUSliceMrl[sell_slices]), sizeof(local_int_t), cudaMemcpyDeviceToHost, stream));

        auto INDEX_TYPE = CUSPARSE_INDEX_32I;
#ifdef INDEX_64 // In src/Geometry
        INDEX_TYPE = CUSPARSE_INDEX_64I;
#endif
        CHECK_CUSPARSE(cusparseCreateSlicedEll(&(A->cusparseOpt.matL), nrow, nrow, half_nnz, sell_l_nnz, slice_size,
            A->sellLSliceMrl, A->sellLPermColumns, A->sellLPermValues, INDEX_TYPE, INDEX_TYPE, CUSPARSE_INDEX_BASE_ZERO,
            CUDA_R_64F));

        CHECK_CUSPARSE(cusparseCreateSlicedEll(&(A->cusparseOpt.matU), nrow, nrow, half_nnz, sell_u_nnz, slice_size,
            A->sellUSliceMrl, A->sellUPermColumns, A->sellUPermValues, INDEX_TYPE, INDEX_TYPE, CUSPARSE_INDEX_BASE_ZERO,
            CUDA_R_64F));

        local_int_t sell_nnz = sell_slices * slice_size * HPCG_MAX_ROW_LEN;
        CHECK_CUSPARSE(cusparseCreateSlicedEll(&(A->cusparseOpt.matA), nrow, nrow, A->localNumberOfNonzeros, sell_nnz,
            slice_size, A->sellASliceMrl, A->sellAPermColumns, A->sellAPermValues, INDEX_TYPE, INDEX_TYPE,
            CUSPARSE_INDEX_BASE_ZERO, CUDA_R_64F));

        double alpha = 1.0, beta = 0.0;
        size_t e_buf_size = 0;
        size_t l_buf_size = 0, u_buf_size = 0, i_buf_size = 0, max_buf_size = 0;
        cusparseDnVecDescr_t dummy1, dummy2;
        CHECK_CUSPARSE(cusparseCreateDnVec(&dummy1, nrow, x.values_d, CUDA_R_64F));
        CHECK_CUSPARSE(cusparseCreateDnVec(&dummy2, nrow, b.values_d, CUDA_R_64F));
        CHECK_CUSPARSE(cusparseCreateDnVec(&(A->cusparseOpt.vecX), nrow, x.values_d, CUDA_R_64F));
        CHECK_CUSPARSE(cusparseCreateDnVec(&(A->cusparseOpt.vecY), nrow, b.values_d, CUDA_R_64F));
        max_buf_size = e_buf_size;

        // MV
        // Lower
        CHECK_CUSPARSE(cusparseSpMV_bufferSize(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha,
            A->cusparseOpt.matL, dummy1, &beta, dummy2, CUDA_R_64F, CUSPARSE_SPMV_ALG_DEFAULT, &l_buf_size));
        CHECK_CUSPARSE(cusparseSpMV_bufferSize(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha,
            A->cusparseOpt.matU, dummy1, &beta, dummy2, CUDA_R_64F, CUSPARSE_SPMV_ALG_DEFAULT, &u_buf_size));
        CHECK_CUSPARSE(cusparseSpMV_bufferSize(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha,
            A->cusparseOpt.matA, dummy1, &beta, dummy2, CUDA_R_64F, CUSPARSE_SPMV_ALG_DEFAULT, &i_buf_size));

        max_buf_size = std::max(std::max(i_buf_size, e_buf_size), std::max(u_buf_size, l_buf_size));

        // SV
        // Lower
        size_t buffer_size_sv_l, buffer_size_sv_u;
        cusparseFillMode_t fillmode_l = CUSPARSE_FILL_MODE_LOWER;
        cusparseFillMode_t fillmode_u = CUSPARSE_FILL_MODE_UPPER;
        cusparseDiagType_t diagtype = CUSPARSE_DIAG_TYPE_NON_UNIT;

        CHECK_CUSPARSE(cusparseSpSV_createDescr(&A->cusparseOpt.spsvDescrL));
        CHECK_CUSPARSE(cusparseSpSV_createDescr(&A->cusparseOpt.spsvDescrU));
        CHECK_CUSPARSE(
            cusparseSpMatSetAttribute(A->cusparseOpt.matL, CUSPARSE_SPMAT_DIAG_TYPE, &(diagtype), sizeof(diagtype)));
        CHECK_CUSPARSE(cusparseSpMatSetAttribute(
            A->cusparseOpt.matL, CUSPARSE_SPMAT_FILL_MODE, &(fillmode_l), sizeof(fillmode_l)));

        if (!Use_Hpcg_Mem_Reduction || (nrow % 8 != 0))
        {
            CHECK_CUSPARSE(cusparseSpSV_bufferSize(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha,
                A->cusparseOpt.matL, A->cusparseOpt.vecX, A->cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPSV_ALG_DEFAULT,
                A->cusparseOpt.spsvDescrL, &buffer_size_sv_l));
            CHECK_CUDART(cudaMalloc(&A->bufferSvL, buffer_size_sv_l));
        }
        CHECK_CUSPARSE(cusparseSpSV_analysis(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha,
            A->cusparseOpt.matL, A->cusparseOpt.vecX, A->cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPSV_ALG_DEFAULT,
            A->cusparseOpt.spsvDescrL, A->bufferSvL));
        CHECK_CUSPARSE(cusparseSpSV_updateMatrix(
            cusparsehandle, A->cusparseOpt.spsvDescrL, A->diagonal, CUSPARSE_SPSV_UPDATE_DIAGONAL));

        CHECK_CUSPARSE(
            cusparseSpMatSetAttribute(A->cusparseOpt.matU, CUSPARSE_SPMAT_FILL_MODE, &(fillmode_u), sizeof(fillmode_u)));

        if (!Use_Hpcg_Mem_Reduction || (nrow % 8 != 0))
        {
            CHECK_CUSPARSE(cusparseSpSV_bufferSize(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha,
                A->cusparseOpt.matU, A->cusparseOpt.vecX, A->cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPSV_ALG_DEFAULT,
                A->cusparseOpt.spsvDescrU, &buffer_size_sv_u));
            CHECK_CUDART(cudaMalloc(&A->bufferSvU, buffer_size_sv_u));
        }
        CHECK_CUSPARSE(cusparseSpSV_analysis(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &alpha,
            A->cusparseOpt.matU, A->cusparseOpt.vecX, A->cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPSV_ALG_DEFAULT,
            A->cusparseOpt.spsvDescrU, A->bufferSvU));
        CHECK_CUSPARSE(cusparseSpSV_updateMatrix(
            cusparsehandle, A->cusparseOpt.spsvDescrU, A->diagonal, CUSPARSE_SPSV_UPDATE_DIAGONAL));

        if (max_buf_size > 0)
            CHECK_CUDART(cudaMalloc(&(A->bufferMvA), max_buf_size));

        CHECK_CUSPARSE(cusparseDestroyDnVec(dummy1));
        CHECK_CUSPARSE(cusparseDestroyDnVec(dummy2));
        // //////////////////////////////////////////////////////////////////////////
        A = A->Ac;
    }

    A = &A_in;
    for (int level = 1; level < numberOfMgLevels; ++level)
    {
        const local_int_t nrow_c = A->Ac->localNumberOfRows;
        const local_int_t nrow_f = A->localNumberOfRows;
        F2cPermCuda(nrow_c, A->gpuAux.f2c, A->f2cPerm, A->ref2opt, A->Ac->opt2ref);
        A = A->Ac;
    }

    return 0;
}
#endif

#ifdef USE_GRACE
size_t OptimizeProblemCpu(SparseMatrix& A_in, CGData& data, Vector& b, Vector& x, Vector& xexact)
{
    // Initialize data structures
    size_t mem = AllocateMemCpu(A_in);

    SparseMatrix* A = &A_in;
    local_int_t numberOfMgLevels = 4;
    local_int_t slice_size = A->slice_size;
    for (int level = 0; level < numberOfMgLevels; ++level)
    {
        // Color the matrix
        int num_colors;
        ColorMatrixCpu(*A, &num_colors);
        A->totalColors = num_colors;

        // Compute when each color starts
        A->cpuAux.firstRowOfColor[0] = 0;
        for (int c = 1; c < A->totalColors; c++)
        {
            A->cpuAux.firstRowOfColor[c] = A->cpuAux.firstRowOfColor[c - 1] + A->cpuAux.nRowsWithColor[c - 1];
        }

        // Reorder the matrix
        CreateSellPermCpu(*A);

#ifndef HPCG_NO_MPI
        // Translate row IDs that will be send to neighbours
#pragma omp parallel for
        for (local_int_t i = 0; i < A->totalToBeSent; i++)
        {
            local_int_t orig = A->elementsToSend[i];
            A->elementsToSend[i] = A->ref2opt[orig];
        }
#endif

        local_int_t numberOfNonzerosPerRow = HPCG_MAX_ROW_LEN;
        local_int_t nrow = A->localNumberOfRows;
        local_int_t half_nnz = (A->localNumberOfNonzeros - nrow - A->extNnz) / 2;
        local_int_t num_slices = (nrow + slice_size - 1) / slice_size;
        local_int_t sell_l_nnz = A->sellLSliceMrl[num_slices];
        local_int_t sell_u_nnz = A->sellUSliceMrl[num_slices];
        local_int_t sell_nnz = num_slices * slice_size * numberOfNonzerosPerRow;

        auto INDEX_TYPE = NVPL_SPARSE_INDEX_32I;
#ifdef INDEX_64 // In src/Geometry
        INDEX_TYPE = NVPL_SPARSE_INDEX_64I;
#endif

        nvpl_sparse_create_sliced_ell(&(A->nvplSparseOpt.matL), nrow, nrow, half_nnz, sell_l_nnz, slice_size,
            A->sellLSliceMrl, A->sellLPermColumns, A->sellLPermValues, INDEX_TYPE, INDEX_TYPE,
            NVPL_SPARSE_INDEX_BASE_ZERO, NVPL_SPARSE_R_64F);

        nvpl_sparse_create_sliced_ell(&(A->nvplSparseOpt.matU), nrow, nrow, half_nnz, sell_u_nnz, slice_size,
            A->sellUSliceMrl, A->sellUPermColumns, A->sellUPermValues, INDEX_TYPE, INDEX_TYPE,
            NVPL_SPARSE_INDEX_BASE_ZERO, NVPL_SPARSE_R_64F);

        nvpl_sparse_create_sliced_ell(&(A->nvplSparseOpt.matA), nrow, nrow, A->localNumberOfNonzeros, sell_nnz,
            slice_size, A->sellASliceMrl, A->sellAPermColumns, A->sellAPermValues, INDEX_TYPE, INDEX_TYPE,
            NVPL_SPARSE_INDEX_BASE_ZERO, NVPL_SPARSE_R_64F);

        double alpha = 1.0, beta = 0.0;
        size_t e_buf_size = 0;
        size_t l_buf_size = 0, u_buf_size = 0, i_buf_size = 0, max_buf_size = 0;
        nvpl_sparse_create_dn_vec(&(A->nvplSparseOpt.vecX), nrow, x.values, NVPL_SPARSE_R_64F);
        nvpl_sparse_create_dn_vec(&(A->nvplSparseOpt.vecY), nrow, b.values, NVPL_SPARSE_R_64F);
        max_buf_size = e_buf_size;

        // //MV
        // //Lower
        nvpl_sparse_spmv_create_descr(&A->nvplSparseOpt.spmvLDescr);
        nvpl_sparse_spmv_buffer_size(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &alpha,
            A->nvplSparseOpt.matL, A->nvplSparseOpt.vecX, &beta, A->nvplSparseOpt.vecY, A->nvplSparseOpt.vecY,
            NVPL_SPARSE_R_64F, NVPL_SPARSE_SPMV_ALG_DEFAULT, A->nvplSparseOpt.spmvLDescr, &l_buf_size);
        // //Upper
        nvpl_sparse_spmv_create_descr(&A->nvplSparseOpt.spmvUDescr);
        nvpl_sparse_spmv_buffer_size(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &alpha,
            A->nvplSparseOpt.matU, A->nvplSparseOpt.vecX, &beta, A->nvplSparseOpt.vecY, A->nvplSparseOpt.vecY,
            NVPL_SPARSE_R_64F, NVPL_SPARSE_SPMV_ALG_DEFAULT, A->nvplSparseOpt.spmvUDescr, &u_buf_size);
        // //L+D+U
        nvpl_sparse_spmv_create_descr(&A->nvplSparseOpt.spmvADescr);
        nvpl_sparse_spmv_buffer_size(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &alpha,
            A->nvplSparseOpt.matA, A->nvplSparseOpt.vecX, &beta, A->nvplSparseOpt.vecY, A->nvplSparseOpt.vecY,
            NVPL_SPARSE_R_64F, NVPL_SPARSE_SPMV_ALG_DEFAULT, A->nvplSparseOpt.spmvADescr, &i_buf_size);

        max_buf_size = std::max(std::max(i_buf_size, e_buf_size), std::max(u_buf_size, l_buf_size));

        // //SV
        // //Lower
        size_t buffer_size_sv_l, buffer_size_sv_u;
        nvpl_sparse_fill_mode_t fillmode_l = NVPL_SPARSE_FILL_MODE_LOWER;
        nvpl_sparse_fill_mode_t fillmode_u = NVPL_SPARSE_FILL_MODE_UPPER;
        nvpl_sparse_diag_type_t diagtype = NVPL_SPARSE_DIAG_TYPE_NON_UNIT;

        nvpl_sparse_spsv_create_descr(&A->nvplSparseOpt.spsvDescrL);
        nvpl_sparse_spsv_create_descr(&A->nvplSparseOpt.spsvDescrU);
        nvpl_sparse_sp_mat_set_attribute(
            A->nvplSparseOpt.matL, NVPL_SPARSE_SPMAT_DIAG_TYPE, &(diagtype), sizeof(diagtype));
        nvpl_sparse_sp_mat_set_attribute(
            A->nvplSparseOpt.matL, NVPL_SPARSE_SPMAT_FILL_MODE, &(fillmode_l), sizeof(fillmode_l));

        Vector origDiagA;
        InitializeVector(origDiagA, A->localNumberOfRows, CPU);
        CopyMatrixDiagonal(*A, origDiagA);

        // Pass strictly L, and then update the diagonal
        if (!Use_Hpcg_Mem_Reduction || A->localNumberOfRows % 8 != 0)
        {
            nvpl_sparse_sp_mat_set_attribute(
                A->nvplSparseOpt.matA, NVPL_SPARSE_SPMAT_FILL_MODE, &(fillmode_l), sizeof(fillmode_l));
            nvpl_sparse_spsv_buffer_size(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &alpha,
                A->nvplSparseOpt.matA, A->nvplSparseOpt.vecX, A->nvplSparseOpt.vecY, NVPL_SPARSE_R_64F,
                NVPL_SPARSE_SPSV_ALG_DEFAULT, A->nvplSparseOpt.spsvDescrL, &buffer_size_sv_l);

            A->bufferSvL = new char[buffer_size_sv_l];
            mem += buffer_size_sv_l;
            nvpl_sparse_spsv_analysis(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &alpha,
                A->nvplSparseOpt.matA, A->nvplSparseOpt.vecX, A->nvplSparseOpt.vecY, NVPL_SPARSE_R_64F,
                NVPL_SPARSE_SPSV_ALG_DEFAULT, A->nvplSparseOpt.spsvDescrL, A->bufferSvL);
        }
        else
        {
            nvpl_sparse_spsv_analysis(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &alpha,
                A->nvplSparseOpt.matL, A->nvplSparseOpt.vecX, A->nvplSparseOpt.vecY, NVPL_SPARSE_R_64F,
                NVPL_SPARSE_SPSV_ALG_DEFAULT, A->nvplSparseOpt.spsvDescrL, A->bufferSvL);
            nvpl_sparse_spsv_update_matrix(
                nvpl_sparse_handle, A->nvplSparseOpt.spsvDescrL, origDiagA.values, NVPL_SPARSE_SPSV_UPDATE_DIAGONAL);
        }

        // Pass strctly U, and then update diagonal
        nvpl_sparse_sp_mat_set_attribute(
            A->nvplSparseOpt.matU, NVPL_SPARSE_SPMAT_FILL_MODE, &(fillmode_u), sizeof(fillmode_u));
        if (!Use_Hpcg_Mem_Reduction || A->localNumberOfRows % 8 != 0)
        {
            nvpl_sparse_sp_mat_set_attribute(
                A->nvplSparseOpt.matA, NVPL_SPARSE_SPMAT_FILL_MODE, &(fillmode_u), sizeof(fillmode_u));
            nvpl_sparse_spsv_buffer_size(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &alpha,
                A->nvplSparseOpt.matA, A->nvplSparseOpt.vecX, A->nvplSparseOpt.vecY, NVPL_SPARSE_R_64F,
                NVPL_SPARSE_SPSV_ALG_DEFAULT, A->nvplSparseOpt.spsvDescrU, &buffer_size_sv_u);
            A->bufferSvU = new char[buffer_size_sv_u];
            mem += buffer_size_sv_u;
            nvpl_sparse_spsv_analysis(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &alpha,
                A->nvplSparseOpt.matA, A->nvplSparseOpt.vecX, A->nvplSparseOpt.vecY, NVPL_SPARSE_R_64F,
                NVPL_SPARSE_SPSV_ALG_DEFAULT, A->nvplSparseOpt.spsvDescrU, A->bufferSvU);
        }
        else
        {
            nvpl_sparse_spsv_analysis(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &alpha,
                A->nvplSparseOpt.matU, A->nvplSparseOpt.vecX, A->nvplSparseOpt.vecY, NVPL_SPARSE_R_64F,
                NVPL_SPARSE_SPSV_ALG_DEFAULT, A->nvplSparseOpt.spsvDescrU, A->bufferSvU);
            nvpl_sparse_spsv_update_matrix(
                nvpl_sparse_handle, A->nvplSparseOpt.spsvDescrU, origDiagA.values, NVPL_SPARSE_SPSV_UPDATE_DIAGONAL);
        }

        DeleteVector(origDiagA);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////

        A = A->Ac;
    }
    A = &A_in;

    for (int level = 1; level < numberOfMgLevels; level++)
    {
        local_int_t nrow_c = A->Ac->localNumberOfRows;
        local_int_t nrow_f = A->localNumberOfRows;
        // Permute space injector operator
        F2cPermCpu(nrow_c, A->mgData->f2cOperator, A->f2cPerm, A->ref2opt, A->Ac->opt2ref);
        A = A->Ac;
    }

    return mem;
}
#endif // USE_GRACE

size_t OptimizeProblem(SparseMatrix& A_in, CGData& data, Vector& b, Vector& x, Vector& xexact)
{
    size_t result = 0;
    if (A_in.rankType == GPU)
    {
#ifdef USE_CUDA
        result = OptimizeProblemGpu(A_in, data, b, x, xexact);
#endif
    }
    else
    {
#ifdef USE_GRACE
        result = OptimizeProblemCpu(A_in, data, b, x, xexact);
#endif
    }

    return result;
}

// Helper function (see OptimizeProblem.hpp for details)
double OptimizeProblemMemoryUse(const SparseMatrix& A)
{

    return 0.0;
}
