
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
 @file TestSymmetry.cpp

 HPCG routine
 */

// The MPI include must be first for Windows platforms
#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif
#include <cfloat>
#include <fstream>
#include <iostream>
using std::endl;
#include <cmath>
#include <vector>

#include "hpcg.hpp"

#include "ComputeDotProduct.hpp"
#include "ComputeMG.hpp"
#include "ComputeResidual.hpp"
#include "ComputeSPMV.hpp"
#include "CpuKernels.hpp"
#include "CudaKernels.hpp"
#include "Geometry.hpp"
#include "SparseMatrix.hpp"
#include "TestSymmetry.hpp"

extern int use_output_file;
/*!
  Tests symmetry-preserving properties of the sparse matrix vector multiply and multi-grid routines.

  @param[in]    geom   The description of the problem's geometry.
  @param[in]    A      The known system matrix
  @param[in]    b      The known right hand side vector
  @param[in]    xexact The exact solution vector
  @param[inout] testsymmetry_data The data structure with the results of the CG symmetry test including pass/fail
  information

  @return returns 0 upon success and non-zero otherwise

  @see ComputeDotProduct
  @see ComputeDotProduct_ref
  @see ComputeSPMV
  @see ComputeSPMV_ref
  @see ComputeMG
  @see ComputeMG_ref
*/
int TestSymmetry(SparseMatrix& A, Vector& b, Vector& xexact, TestSymmetryData& testsymmetry_data)
{

    local_int_t nrow = A.localNumberOfRows;
    local_int_t ncol = A.localNumberOfColumns;
    Vector x_ncol, y_ncol, z_ncol;
    InitializeVector(x_ncol, ncol, A.rankType);
    InitializeVector(y_ncol, ncol, A.rankType);
    InitializeVector(z_ncol, ncol, A.rankType);

    double t4 = 0.0; // Needed for dot-product call, otherwise unused
    testsymmetry_data.count_fail = 0;

    // Test symmetry of matrix
    // First load vectors with random values
    FillRandomVector(x_ncol);
    FillRandomVector(y_ncol);

    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
        CopyVectorH2D(y_ncol);
        CopyVectorH2D(x_ncol);
#endif
    }
    int ierr;

    double xNorm2, yNorm2;
    double ANorm = 2 * 26.0;

    // Next, compute x'*A*y
    ComputeDotProduct(nrow, y_ncol, y_ncol, yNorm2, t4, A.isDotProductOptimized, A.rankType);
    ierr = ComputeSPMV(A, y_ncol, z_ncol); // z_nrow = A*y_overlap
    if (ierr)
        if (use_output_file)
        {
            HPCG_fout << "Error in call to SpMV: " << ierr << ".\n" << endl;
        }
        else
        {
            std::cout << "Error in call to SpMV: " << ierr << ".\n" << endl;
        }
    double xtAy = 0.0;
    ierr = ComputeDotProduct(nrow, x_ncol, z_ncol, xtAy, t4, A.isDotProductOptimized, A.rankType); // x'*A*y
    if (ierr)
        if (use_output_file)
        {
            HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
        }
        else
        {
            std::cout << "Error in call to dot: " << ierr << ".\n" << endl;
        }

    // Next, compute y'*A*x
    ComputeDotProduct(nrow, x_ncol, x_ncol, xNorm2, t4, A.isDotProductOptimized, A.rankType);
    ierr = ComputeSPMV(A, x_ncol, z_ncol); // b_computed = A*x_overlap
    if (ierr)
        if (use_output_file)
        {
            HPCG_fout << "Error in call to SpMV: " << ierr << ".\n" << endl;
        }
        else
        {
            std::cout << "Error in call to SpMV: " << ierr << ".\n" << endl;
        }
    double ytAx = 0.0;
    ierr = ComputeDotProduct(nrow, y_ncol, z_ncol, ytAx, t4, A.isDotProductOptimized, A.rankType); // y'*A*x
    if (ierr)
        if (use_output_file)
        {
            HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
        }
        else
        {
            std::cout << "Error in call to dot: " << ierr << ".\n" << endl;
        }

    testsymmetry_data.depsym_spmv = std::fabs((long double) (xtAy - ytAx))
        / ((xNorm2 * ANorm * yNorm2 + yNorm2 * ANorm * xNorm2) * (DBL_EPSILON));
    if (testsymmetry_data.depsym_spmv > 1.0)
        ++testsymmetry_data.count_fail; // If the difference is > 1, count it wrong
    if (A.geom->rank == 0)
        if (use_output_file)
        {
            HPCG_fout << "Departure from symmetry (scaled) for SpMV abs(x'*A*y - y'*A*x) = "
                      << testsymmetry_data.depsym_spmv << endl;
        }
        else
        {
            std::cout << "Departure from symmetry (scaled) for SpMV abs(x'*A*y - y'*A*x) = "
                      << testsymmetry_data.depsym_spmv << endl;
        }

    // Test symmetry of multi-grid
    // Compute x'*Minv*y
    ierr = ComputeMG(A, y_ncol, z_ncol); // z_ncol = Minv*y_ncol
    if (ierr)
        if (use_output_file)
        {
            HPCG_fout << "Error in call to MG: " << ierr << ".\n" << endl;
        }
        else
        {
            std::cout << "Error in call to MG: " << ierr << ".\n" << endl;
        }
    double xtMinvy = 0.0;
    ierr = ComputeDotProduct(nrow, x_ncol, z_ncol, xtMinvy, t4, A.isDotProductOptimized, A.rankType); // x'*Minv*y
    if (ierr)
        if (use_output_file)
        {
            HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
        }
        else
        {
            std::cout << "Error in call to dot: " << ierr << ".\n" << endl;
        }
    // Next, compute z'*Minv*x
    ierr = ComputeMG(A, x_ncol, z_ncol); // z_ncol = Minv*x_ncol
    if (ierr)
        if (use_output_file)
        {
            HPCG_fout << "Error in call to MG: " << ierr << ".\n" << endl;
        }
        else
        {
            std::cout << "Error in call to MG: " << ierr << ".\n" << endl;
        }
    double ytMinvx = 0.0;
    ierr = ComputeDotProduct(nrow, y_ncol, z_ncol, ytMinvx, t4, A.isDotProductOptimized, A.rankType); // y'*Minv*x
    if (ierr)
        if (use_output_file)
        {
            HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
        }
        else
        {
            std::cout << "Error in call to dot: " << ierr << ".\n" << endl;
        }
    testsymmetry_data.depsym_mg = std::fabs((long double) (xtMinvy - ytMinvx))
        / ((xNorm2 * ANorm * yNorm2 + yNorm2 * ANorm * xNorm2) * (DBL_EPSILON));
    if (testsymmetry_data.depsym_mg > 1.0)
        ++testsymmetry_data.count_fail; // If the difference is > 1, count it wrong
    if (A.geom->rank == 0)
        if (use_output_file)
        {
            HPCG_fout << "Departure from symmetry (scaled) for MG abs(x'*Minv*y - y'*Minv*x) = "
                      << testsymmetry_data.depsym_mg << endl;
        }
        else
        {
            std::cout << "Departure from symmetry (scaled) for MG abs(x'*Minv*y - y'*Minv*x) = "
                      << testsymmetry_data.depsym_mg << endl;
        }

    CopyVector(xexact, x_ncol); // Copy exact answer into overlap vector

    int numberOfCalls = 2;
    double residual = 0.0;
    for (int i = 0; i < numberOfCalls; ++i)
    {
        if (A.rankType == GPU)
        {
#ifdef USE_CUDA
            CopyVectorH2D(x_ncol);
#endif
        }

        ierr = ComputeSPMV(A, x_ncol, z_ncol); // b_computed = A*x_overlap

        if (A.rankType == GPU)
        {
#ifdef USE_CUDA
            PermVectorCuda(A.ref2opt, z_ncol, nrow);
            CopyVectorD2H(z_ncol);
#endif
        }
        else
        {
#ifdef USE_GRACE
            PermVectorCpu(A.ref2opt, z_ncol, nrow);
#endif
        }

        if (ierr)
            if (use_output_file)
            {
                HPCG_fout << "Error in call to SpMV: " << ierr << ".\n" << endl;
            }
            else
            {
                std::cout << "Error in call to SpMV: " << ierr << ".\n" << endl;
            }
        if ((ierr = ComputeResidual(A.localNumberOfRows, b, z_ncol, residual)))
            if (use_output_file)
            {
                HPCG_fout << "Error in call to compute_residual: " << ierr << ".\n" << endl;
            }
            else
            {
                std::cout << "Error in call to compute_residual: " << ierr << ".\n" << endl;
            }
        if (A.geom->rank == 0)
            if (use_output_file)
            {
                HPCG_fout << "SpMV call [" << i << "] Residual [" << residual << "]" << endl;
            }
            else
            {
                std::cout << "SpMV call [" << i << "] Residual [" << residual << "]" << endl;
            }
    }
    DeleteVector(x_ncol);
    DeleteVector(y_ncol);
    DeleteVector(z_ncol);

    return 0;
}
