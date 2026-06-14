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

#ifndef CPUKERNELS_HPP
#define CPUKERNELS_HPP

#ifdef USE_GRACE

#include <nvpl_sparse.h>
extern nvpl_sparse_handle_t nvpl_sparse_handle;

#include "SparseMatrix.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <random>
#include <vector>

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif

///////// Deallocate CPU Memory for data structures //
void DeleteMatrixCpu(SparseMatrix& A);

///////// Find the size of CPU reference allocated memory //
size_t EstimateCpuRefMem(SparseMatrix& A);

/*
    Translation of a 3D point in all directions
    27 possibilities
*/
constexpr int tid2indCpu[32][4] = {{-1, -1, -1, 0}, {0, -1, -1, 0}, {1, -1, -1, 0}, {-1, 0, -1, 0}, {0, 0, -1, 0},
    {1, 0, -1, 0}, {-1, 1, -1, 0}, {0, 1, -1, 0}, {1, 1, -1, 0}, {-1, -1, 0, 0}, {0, -1, 0, 0}, {1, -1, 0, 0},
    {-1, 0, 0, 0}, {0, 0, 0, 0}, {1, 0, 0, 0}, {-1, 1, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {-1, -1, 1, 0}, {0, -1, 1, 0},
    {1, -1, 1, 0}, {-1, 0, 1, 0}, {0, 0, 1, 0}, {1, 0, 1, 0}, {-1, 1, 1, 0}, {0, 1, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0},
    {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

// Generate Problem
// Inclusive Prefix Sum
void PrefixsumCpu(int* x, int N);

// Optimize Problem
size_t AllocateMemCpu(SparseMatrix& A_in);
void ColorMatrixCpu(SparseMatrix& A, int* num_colors);
void CreateSellPermCpu(SparseMatrix& A);
void F2cPermCpu(local_int_t nrow_c, local_int_t* f2c, local_int_t* f2c_perm, local_int_t* perm_f, local_int_t* iperm_c);

// Permute a vector using coloring buffer
void PermVectorCpu(local_int_t* perm, Vector& x, local_int_t length);

// Test CG
void ReplaceMatrixDiagonalCpu(SparseMatrix& A, Vector diagonal);

// CG Support Kernels
// Dot-product Per single rank
void ComputeDotProductCpu(const local_int_t n, const Vector& x, const Vector& y, double& result, bool& isOptimized);

// WAXPBY
int ComputeWAXPBYCpu(const local_int_t n, const double alpha, const Vector& x, const double beta, const Vector& y,
    Vector& w, bool& isOptimized);
// SYMGS
void SpmvDiagCpu(local_int_t n, const double* x, double* y, double* z);
void AxpbyCpu(local_int_t n, double* x, double* y, double* z);
void SpFmaCpu(local_int_t n, const double* x, double* y, double* z);

// External Matrix SpMV + Scatter
void ExtSpMVCpu(const SparseMatrix& A, const local_int_t n, const double alpha, const double* x, double* y);

#endif // USE_GRACE
#endif // CPUKERNELS_HPP