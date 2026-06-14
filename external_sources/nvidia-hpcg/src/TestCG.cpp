
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
 @file TestCG.cpp

 HPCG routine
 */

// Changelog
//
// Version 0.4
// - Added timing of setup time for sparse MV
// - Corrected percentages reported for sparse MV with overhead
//
/////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
using std::endl;
#include "hpcg.hpp"
#include <vector>

#include "CG.hpp"
#include "CG_ref.hpp"
#include "TestCG.hpp"
#include "CpuKernels.hpp"
#include "CudaKernels.hpp"

extern int use_output_file;

/*!
  Test the correctness of the Preconditined CG implementation by using a system matrix with a dominant diagonal.

  @param[in]    geom The description of the problem's geometry.
  @param[in]    A    The known system matrix
  @param[in]    data the data structure with all necessary CG vectors preallocated
  @param[in]    b    The known right hand side vector
  @param[inout] x    On entry: the initial guess; on exit: the new approximate solution
  @param[out]   testcg_data the data structure with the results of the test including pass/fail information

  @return Returns zero on success and a non-zero value otherwise.

  @see CG()
 */

int TestCG(SparseMatrix& A, CGData& data, Vector& b, Vector& x, TestCGData& testcg_data)
{
    // Use this array for collecting timing information
    std::vector<double> times(8, 0.0);
    // Temporary storage for holding original diagonal and RHS
    Vector origDiagA, exaggeratedDiagA, origB;
    InitializeVector(origDiagA, A.localNumberOfRows, A.rankType);
    InitializeVector(exaggeratedDiagA, A.localNumberOfRows, A.rankType);
    InitializeVector(origB, A.localNumberOfRows, A.rankType);
    CopyMatrixDiagonal(A, origDiagA);
    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
        CopyMatrixDiagonalCuda(A, origDiagA);
#endif
    }
    CopyVector(origDiagA, exaggeratedDiagA);
    CopyVector(b, origB);

    // Modify the matrix diagonal to greatly exaggerate diagonal values.
    // CG should converge in about 10 iterations for this problem, regardless of problem size
    for (local_int_t i = 0; i < A.localNumberOfRows; ++i)
    {
        global_int_t globalRowID = A.localToGlobalMap[i];
        if (globalRowID < 9)
        {
            double scale = (globalRowID + 2) * 1.0e6;
            ScaleVectorValue(exaggeratedDiagA, i, scale);
            ScaleVectorValue(b, i, scale);
        }
        else
        {
            ScaleVectorValue(exaggeratedDiagA, i, 1.0e6);
            ScaleVectorValue(b, i, 1.0e6);
        }
    }

    // Reference Matrix
    ReplaceMatrixDiagonal(A, exaggeratedDiagA);

    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
        CopyVectorH2D(exaggeratedDiagA);
        PermVectorCuda(A.opt2ref, b, A.localNumberOfRows);
        PermVectorCuda(A.opt2ref, exaggeratedDiagA, A.localNumberOfRows);
        ReplaceMatrixDiagonalCuda(A, exaggeratedDiagA);
        CHECK_CUSPARSE(cusparseSpSV_updateMatrix(
            cusparsehandle, A.cusparseOpt.spsvDescrL, exaggeratedDiagA.values_d, CUSPARSE_SPSV_UPDATE_DIAGONAL));
        CHECK_CUSPARSE(cusparseSpSV_updateMatrix(
            cusparsehandle, A.cusparseOpt.spsvDescrU, exaggeratedDiagA.values_d, CUSPARSE_SPSV_UPDATE_DIAGONAL));
#endif
    }
    else
    {
#ifdef USE_GRACE
        PermVectorCpu(A.opt2ref, b, A.localNumberOfRows);
        PermVectorCpu(A.opt2ref, exaggeratedDiagA, A.localNumberOfRows);
        ReplaceMatrixDiagonalCpu(A, exaggeratedDiagA);
        nvpl_sparse_spsv_update_matrix(
            nvpl_sparse_handle, A.nvplSparseOpt.spsvDescrL, exaggeratedDiagA.values, NVPL_SPARSE_SPSV_UPDATE_DIAGONAL);
        nvpl_sparse_spsv_update_matrix(
            nvpl_sparse_handle, A.nvplSparseOpt.spsvDescrU, exaggeratedDiagA.values, NVPL_SPARSE_SPSV_UPDATE_DIAGONAL);
#endif
    }

    ////////////////////////////////

    int niters = 0;
    double normr = 0.0;
    double normr0 = 0.0;
    int maxIters = 50;
    int numberOfCgCalls = 2;
    double tolerance = 1.0e-12; // Set tolerance to reasonable value for grossly scaled diagonal terms
    testcg_data.expected_niters_no_prec
        = 12; // For the unpreconditioned CG call, we should take about 10 iterations, permit 12
    testcg_data.expected_niters_prec = 2; // For the preconditioned case, we should take about 1 iteration, permit 2
    testcg_data.niters_max_no_prec = 0;
    testcg_data.niters_max_prec = 0;
    for (int k = 0; k < 2; ++k)
    { // This loop tests both unpreconditioned and preconditioned runs
        int expected_niters = testcg_data.expected_niters_no_prec;
        if (k == 1)
            expected_niters = testcg_data.expected_niters_prec;
        for (int i = 0; i < numberOfCgCalls; ++i)
        {
            ZeroVector(x); // Zero out x
            int ierr = CG(A, data, b, x, maxIters, tolerance, niters, normr, normr0, &times[0], k == 1, 0);
            if (ierr)
                if (use_output_file)
                {
                    HPCG_fout << "Error in call to CG: " << ierr << ".\n" << endl;
                }
                else
                {
                    std::cout << "Error in call to CG: " << ierr << ".\n" << endl;
                }
            if (niters <= expected_niters)
            {
                ++testcg_data.count_pass;
            }
            else
            {
                ++testcg_data.count_fail;
            }
            if (k == 0 && niters > testcg_data.niters_max_no_prec)
                testcg_data.niters_max_no_prec = niters; // Keep track of largest iter count
            if (k == 1 && niters > testcg_data.niters_max_prec)
                testcg_data.niters_max_prec = niters; // Same for preconditioned run
            if (A.geom->rank == 0)
            {
                if (use_output_file)
                {
                    HPCG_fout << "Call [" << i << "] Number of Iterations [" << niters << "] Scaled Residual ["
                              << normr / normr0 << "]" << endl;
                }
                else
                {
                    std::cout << "Call [" << i << "] Number of Iterations [" << niters << "] Scaled Residual ["
                              << normr / normr0 << "]" << endl;
                }
                if (niters > expected_niters)
                    if (use_output_file)
                    {
                        HPCG_fout << " Expected " << expected_niters << " iterations.  Performed " << niters << "."
                                  << endl;
                    }
                    else
                    {
                        std::cout << " Expected " << expected_niters << " iterations.  Performed " << niters << "."
                                  << endl;
                    }
            }
        }
    }

    // Restore matrix diagonal and RHS
    ReplaceMatrixDiagonal(A, origDiagA);

    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
        ReplaceMatrixDiagonalCuda(A, origDiagA);
        CHECK_CUSPARSE(cusparseSpSV_updateMatrix(
            cusparsehandle, A.cusparseOpt.spsvDescrL, origDiagA.values_d, CUSPARSE_SPSV_UPDATE_DIAGONAL));
        CHECK_CUSPARSE(cusparseSpSV_updateMatrix(
            cusparsehandle, A.cusparseOpt.spsvDescrU, origDiagA.values_d, CUSPARSE_SPSV_UPDATE_DIAGONAL));
#endif
    }
    else
    {
#ifdef USE_GRACE
        ReplaceMatrixDiagonalCpu(A, origDiagA);
        nvpl_sparse_spsv_update_matrix(
            nvpl_sparse_handle, A.nvplSparseOpt.spsvDescrL, origDiagA.values, NVPL_SPARSE_SPSV_UPDATE_DIAGONAL);
        nvpl_sparse_spsv_update_matrix(
            nvpl_sparse_handle, A.nvplSparseOpt.spsvDescrU, origDiagA.values, NVPL_SPARSE_SPSV_UPDATE_DIAGONAL);
#endif
    }

    CopyVector(origB, b);
    // Delete vectors
    DeleteVector(origDiagA);
    DeleteVector(exaggeratedDiagA);
    DeleteVector(origB);
    testcg_data.normr = normr;

    return 0;
}