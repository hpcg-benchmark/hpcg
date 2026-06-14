
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
 @file ComputeMG.cpp

 HPCG routine
 */

#include "ComputeMG.hpp"
#include "ComputeProlongation.hpp"
#include "ComputeRestriction.hpp"
#include "ComputeSYMGS.hpp"
#include "CudaKernels.hpp"

/*!
  @param[in] A the known system matrix
  @param[in] r the input vector
  @param[inout] x On exit contains the result of the multigrid V-cycle with r as the RHS, x is the approximation to Ax =
  r.

  @return returns 0 upon success and non-zero otherwise

  @see ComputeMG_ref
*/

int ComputeMG(const SparseMatrix& A, const Vector& r, Vector& x)
{
    int ierr = 0;
    if (A.mgData != 0)
    { // Go to next coarse level if defined
        ComputeSYMGS(A, r, x, 1);
        if (A.rankType == GPU)
        {
#ifdef USE_CUDA
            ComputeRestrictionCuda(A, r);
#endif
        }
        else
        {
#ifdef USE_GRACE
            ComputeRestriction(A, r);
#endif
        }

        ierr = ComputeMG(*A.Ac, *A.mgData->rc, *A.mgData->xc);

        if (A.rankType == GPU)
        {
#ifdef USE_CUDA
            ComputeProlongationCuda(A, x);
#endif
        }
        else
        {
#ifdef USE_GRACE
            ComputeProlongation(A, x);
#endif
        }

        ComputeSYMGS(A, r, x, 0);
    }
    else
    {
        ComputeSYMGS(A, r, x, 1);
    }
    return 0;
}