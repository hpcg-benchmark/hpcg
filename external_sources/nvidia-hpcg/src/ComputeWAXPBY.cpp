
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
 @file ComputeWAXPBY.cpp

 HPCG routine
 */
#ifndef HPCG_NO_MPI
#include "mytimer.hpp"
#include <mpi.h>
#endif
#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif
#ifdef USE_CUDA
#include "Cuda.hpp"
#endif
#include "ComputeWAXPBY.hpp"
#include "ComputeWAXPBY_ref.hpp"
#include "CpuKernels.hpp"
#include "CudaKernels.hpp"
#include "SparseMatrix.hpp"

/*!
  Routine to compute the update of a vector with the sum of two
  scaled vectors where: w = alpha*x + beta*y

  This routine calls the reference WAXPBY implementation by default, but
  can be replaced by a custom, optimized routine suited for
  the target system.

  @param[in] n the number of vector elements (on this processor)
  @param[in] alpha, beta the scalars applied to x and y respectively.
  @param[in] x, y the input vectors
  @param[out] w the output vector
  @param[out] isOptimized should be set to false if this routine uses the reference implementation (is not optimized);
  otherwise leave it unchanged

  @return returns 0 upon success and non-zero otherwise

  @see ComputeWAXPBY_ref
*/

int ComputeWAXPBY(const local_int_t n, const double alpha, const Vector& x, const double beta, const Vector& y,
    Vector& w, bool& isOptimized, rank_type_t rt)
{
    if (rt == GPU)
    {
#ifdef USE_CUDA
        ComputeWAXPBYCuda(n, alpha, x, beta, y, w);
#endif
    }
    else
    {
#ifdef USE_GRACE
        ComputeWAXPBYCpu(n, alpha, x, beta, y, w, isOptimized);
#endif
    }
    return 0;
}
