
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
 @file CG.cpp

 HPCG routine
 */

#include <fstream>

#include <cmath>

#include "hpcg.hpp"

#include "CG.hpp"
#include "ComputeDotProduct.hpp"
#include "ComputeMG.hpp"
#include "ComputeSPMV.hpp"
#include "ComputeWAXPBY.hpp"
#include "mytimer.hpp"
#include <iostream>

#include "CpuKernels.hpp"

#include <mpi.h>

extern int use_output_file;

#define TICKD() t0 = mytimer()       //!< record current time in 't0'
#define TOCKD(t) t += mytimer() - t0 //!< store time difference in 't' using time in 't0'

/*!
  Routine to compute an approximate solution to Ax = b

  @param[in]    geom The description of the problem's geometry.
  @param[inout] A    The known system matrix
  @param[inout] data The data structure with all necessary CG vectors preallocated
  @param[in]    b    The known right hand side vector
  @param[inout] x    On entry: the initial guess; on exit: the new approximate solution
  @param[in]    max_iter  The maximum number of iterations to perform, even if tolerance is not met.
  @param[in]    tolerance The stopping criterion to assert convergence: if norm of residual is <= to tolerance.
  @param[out]   niters    The number of iterations actually performed.
  @param[out]   normr     The 2-norm of the residual vector after the last iteration.
  @param[out]   normr0    The 2-norm of the residual vector before the first iteration.
  @param[out]   times     The 7-element vector of the timing information accumulated during all of the iterations.
  @param[in]    doPreconditioning The flag to indicate whether the preconditioner should be invoked at each iteration.

  @return Returns zero on success and a non-zero value otherwise.

  @see CG_ref()
*/
int CG(const SparseMatrix& A, CGData& data, const Vector& b, Vector& x, const int max_iter, const double tolerance,
    int& niters, double& normr, double& normr0, double* times, bool doPreconditioning, int flag)
{

    double t_begin = mytimer(); // Start timing right away
    normr = 0.0;
    double rtz = 0.0, oldrtz = 0.0, alpha = 0.0, beta = 0.0, pAp = 0.0;
    double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0;
    // #ifndef HPCG_NO_MPI
    //   double t6 = 0.0;
    // #endif
    local_int_t nrow = A.localNumberOfRows;
    Vector& r = data.r; // Residual vector
    Vector& z = data.z; // Preconditioned residual vector
    Vector& p = data.p; // Direction vector (in MPI mode ncol>=nrow)
    Vector& Ap = data.Ap;

    if (!doPreconditioning && A.geom->rank == 0)
        if (use_output_file)
        {
            HPCG_fout << "WARNING: PERFORMING UNPRECONDITIONED ITERATIONS" << std::endl;
        }
        else
        {
            std::cout << "WARNING: PERFORMING UNPRECONDITIONED ITERATIONS" << std::endl;
        }

    int print_freq = 1;
    if (print_freq > 50)
        print_freq = 50;
    if (print_freq < 1)
        print_freq = 1;

    // p is of length ncols, copy x to p for sparse MV operation
    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
        CopyVectorD2D(x, p);
#endif
    }
    else
    {
        CopyVector(x, p);
    }

    TICKD();
    ComputeSPMV(A, p, Ap);
    TOCKD(t3); // Ap = A*p
    TICKD();
    ComputeWAXPBY(nrow, 1.0, b, -1.0, Ap, r, A.isWaxpbyOptimized, A.rankType);
    TOCKD(t2); // r = b - Ax (x stored in p)
    TICKD();
    ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized, A.rankType);
    TOCKD(t1);
    normr = sqrt(normr);

    if (A.geom->rank == 0 && flag)
        if (use_output_file)
        {
            HPCG_fout << "Initial Residual = " << normr << std::endl;
        }
        else
        {
            std::cout << "Initial Residual = " << normr << std::endl;
        }

    // Record initial residual for convergence testing
    normr0 = normr;

    // Start iterations
    for (int k = 1; k <= max_iter && normr / normr0 * (1.0 + 1.0e-6) > tolerance; k++)
    {
        TICKD();
        if (doPreconditioning)
        {
            ComputeMG(A, r, z); // Apply preconditioner
            if (A.rankType == GPU)
            {
#ifdef USE_CUDA
                CHECK_CUDART(cudaStreamSynchronize(stream));
#endif
            }
        }
        else
        {
            if (A.rankType == GPU)
            {
#ifdef USE_CUDA
                CopyVectorD2D(r, z); // copy r to z (no preconditioning)
#endif
            }
            else
            {
                CopyVector(r, z); // copy r to z (no preconditioning)
            }
        }
        TOCKD(t5); // Preconditioner apply time

        if (k == 1)
        {
            TICKD();
            ComputeWAXPBY(nrow, 1.0, z, 0.0, z, p, A.isWaxpbyOptimized, A.rankType);
            TOCKD(t2); // Copy Mr to p
            TICKD();
            ComputeDotProduct(nrow, r, z, rtz, t4, A.isDotProductOptimized, A.rankType);
            TOCKD(t1); // rtz = r'*z
        }
        else
        {
            oldrtz = rtz;
            TICKD();
            ComputeDotProduct(nrow, r, z, rtz, t4, A.isDotProductOptimized, A.rankType);
            TOCKD(t1); // rtz = r'*z
            beta = rtz / oldrtz;
            TICKD();
            ComputeWAXPBY(nrow, 1.0, z, beta, p, p, A.isWaxpbyOptimized, A.rankType);
            TOCKD(t2); // p = beta*p + z
        }
        TICKD();
        ComputeSPMV(A, p, Ap);
        TOCKD(t3); // Ap = A*p
        TICKD();
        ComputeDotProduct(nrow, p, Ap, pAp, t4, A.isDotProductOptimized, A.rankType);
        TOCKD(t1); // alpha = p'*Ap
        alpha = rtz / pAp;

        TICKD();
        ComputeWAXPBY(nrow, 1.0, x, alpha, p, x, A.isWaxpbyOptimized, A.rankType); // x = x + alpha*p
        ComputeWAXPBY(nrow, 1.0, r, -alpha, Ap, r, A.isWaxpbyOptimized, A.rankType);
        TOCKD(t2); // r = r - alpha*Ap
        TICKD();
        ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized, A.rankType);
        TOCKD(t1);

        normr = sqrt(normr);

        if (flag && A.geom->rank == 0 && (k % print_freq == 0 || k == max_iter))
            if (use_output_file)
            {
                HPCG_fout << "Iteration = " << k << "   Scaled Residual = " << normr / normr0 << std::endl;
            }
            else
            {
                std::cout << "Iteration = " << k << "   Scaled Residual = " << normr / normr0 << std::endl;
            }

        niters = k;
    }

    // Store times
    times[1] += t1; // dot-product time
    times[2] += t2; // WAXPBY time
    times[3] += t3; // SPMV time
    times[4] += t4; // AllReduce time
    times[5] += t5; // preconditioner apply time
                    // #ifndef HPCG_NO_MPI
    //   times[6] += t6; // exchange halo time
    // #endif
    times[0] += mytimer() - t_begin; // Total time. All done...
    return 0;
}
