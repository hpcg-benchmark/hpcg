
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
 @file GenerateProblem.cpp

 HPCG routine
 */

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif

#include "mytimer.hpp"

#include "GenerateProblem.hpp"
#include "GenerateProblem_ref.hpp"
#ifdef USE_CUDA
#include "Cuda.hpp"
#include "CudaKernels.hpp"
#endif

#ifdef USE_GRACE
#include "CpuKernels.hpp"
#endif

/*!
  Routine to generate a sparse matrix, right hand side, initial guess, and exact solution.

  @param[in]  A        The generated system matrix
  @param[inout] b      The newly allocated and generated right hand side vector (if b!=0 on entry)
  @param[inout] x      The newly allocated solution vector with entries set to 0.0 (if x!=0 on entry)
  @param[inout] xexact The newly allocated solution vector with entries set to the exact solution (if the xexact!=0
  non-zero on entry)

  @see GenerateGeometry
*/
#ifdef USE_CUDA
void GenerateProblem_Gpu(SparseMatrix& A, Vector* b, Vector* x, Vector* xexact)
{
    global_int_t nx = A.geom->nx;
    global_int_t ny = A.geom->ny;
    global_int_t nz = A.geom->nz;
    global_int_t gnx = A.geom->gnx;
    global_int_t gny = A.geom->gny;
    global_int_t gnz = A.geom->gnz;
    global_int_t gix0 = A.geom->gix0;
    global_int_t giy0 = A.geom->giy0;
    global_int_t giz0 = A.geom->giz0;

    local_int_t localNumberOfRows = nx * ny * nz;
    local_int_t numberOfNonzerosPerRow = 27;
    global_int_t totalNumberOfRows = gnx * gny * gnz;

    if (b != 0)
        InitializeVector(*b, localNumberOfRows, GPU);
    if (x != 0)
        InitializeVector(*x, localNumberOfRows, GPU);
    if (xexact != 0)
        InitializeVector(*xexact, localNumberOfRows, GPU);

    GenerateProblemCuda(A, b, x, xexact);

    local_int_t localNumberOfNonzeros = A.localNumberOfNonzeros;
    global_int_t totalNumberOfNonzeros = 27LL * ((gnx - 2LL) * (gny - 2LL) * (gnz - 2LL))
        + 18LL
            * (2LL * ((gnx - 2LL) * (gny - 2LL)) + 2LL * ((gnx - 2LL) * (gnz - 2LL))
                + 2LL * ((gny - 2LL) * (gnz - 2LL)))
        + 12LL * (4LL * (gnx - 2LL) + 4LL * (gny - 2LL) + 4LL * (gnz - 2LL)) + 8LL * 8LL;

    A.title = 0;
    A.totalNumberOfRows = totalNumberOfRows;
    A.totalNumberOfNonzeros = totalNumberOfNonzeros;
    A.localNumberOfRows = localNumberOfRows;
    A.localNumberOfColumns = localNumberOfRows;
    A.localNumberOfNonzeros = localNumberOfNonzeros;

    return;
}
#endif

#ifdef USE_GRACE
// Neighbor rank to sequential ID and vice versa
extern int *rankToId_h, *idToRank_h;
// GenerateProblem_Cpu is called 4 times for each level
// Sometimes we need to perform actions based on the level (global across the applications)
int global_steps = 0;
void GenerateProblem_Cpu(SparseMatrix& A, Vector* b, Vector* x, Vector* xexact)
{
    // Make local copies of geometry information.  Use global_int_t since the RHS products in the calculations
    // below may result in global range values.
    global_int_t nx = A.geom->nx;
    global_int_t ny = A.geom->ny;
    global_int_t nz = A.geom->nz;
    global_int_t gnx = A.geom->gnx;
    global_int_t gny = A.geom->gny;
    global_int_t gnz = A.geom->gnz;
    global_int_t gix0 = A.geom->gix0;
    global_int_t giy0 = A.geom->giy0;
    global_int_t giz0 = A.geom->giz0;
    int npx = A.geom->npx;
    int npy = A.geom->npy;

    local_int_t localNumberOfRows = nx * ny * nz; // This is the size of our subblock
    // If this assert fails, it most likely means that the local_int_t is set to int and should be set to long long
    assert(localNumberOfRows
        > 0); // Throw an exception of the number of rows is less than zero (can happen if int overflow)
    local_int_t numberOfNonzerosPerRow
        = 27; // We are approximating a 27-point finite element/volume/difference 3D stencil

    global_int_t totalNumberOfRows = gnx * gny * gnz; // Total number of grid points in mesh
    // If this assert fails, it most likely means that the global_int_t is set to int and should be set to long long
    assert(totalNumberOfRows
        > 0); // Throw an exception of the number of rows is less than zero (can happen if int overflow)

    // Allocate arrays that are of length localNumberOfRows
    if (global_steps == 0)
    {
        rankToId_h = new int[A.geom->size + 1];
        idToRank_h = new int[27];
        global_steps++;
    }
    local_int_t* nonzerosInRow = new local_int_t[localNumberOfRows];
    global_int_t** mtxIndG = new global_int_t*[localNumberOfRows];
    local_int_t** mtxIndL = new local_int_t*[localNumberOfRows];
    double** matrixValues = new double*[localNumberOfRows];
    double** matrixDiagonal = new double*[localNumberOfRows];

    if (b != 0)
        InitializeVector(*b, localNumberOfRows, CPU);
    if (x != 0)
        InitializeVector(*x, localNumberOfRows, CPU);
    if (xexact != 0)
        InitializeVector(*xexact, localNumberOfRows, CPU);
    double* bv = 0;
    double* xv = 0;
    double* xexactv = 0;
    if (b != 0)
        bv = b->values; // Only compute exact solution if requested
    if (x != 0)
        xv = x->values; // Only compute exact solution if requested
    if (xexact != 0)
        xexactv = xexact->values; // Only compute exact solution if requested
    A.localToGlobalMap.resize(localNumberOfRows);

    // Use a parallel loop to do initial assignment:
    // distributes the physical placement of arrays of pointers across the memory system
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
    for (local_int_t i = 0; i < localNumberOfRows; ++i)
    {
        matrixValues[i] = 0;
        matrixDiagonal[i] = 0;
        mtxIndG[i] = 0;
        mtxIndL[i] = 0;
    }

    if (global_steps == 1)
    {
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
        for (local_int_t i = 0; i < A.geom->size + 1; i++)
        {
            rankToId_h[i] = 0;
        }
        global_steps++;
    }

    // Now allocate the arrays pointed to
    mtxIndL[0] = new local_int_t[localNumberOfRows * numberOfNonzerosPerRow];
    matrixValues[0] = new double[localNumberOfRows * numberOfNonzerosPerRow];
    mtxIndG[0] = new global_int_t[localNumberOfRows * numberOfNonzerosPerRow];

    local_int_t localNumberOfNonzeros = 0;
    local_int_t ext_nnz = 0;
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for reduction(+ : localNumberOfNonzeros) reduction(+ : ext_nnz)
#endif
    for (local_int_t i = 0; i < localNumberOfRows; i++)
    {
        mtxIndL[i] = mtxIndL[0] + i * numberOfNonzerosPerRow;
        matrixValues[i] = matrixValues[0] + i * numberOfNonzerosPerRow;
        mtxIndG[i] = mtxIndG[0] + i * numberOfNonzerosPerRow;

        const local_int_t iz = (i / (nx * ny));
        const local_int_t iy = (i - iz * nx * ny) / nx;
        const local_int_t ix = i - (iz * ny + iy) * nx;
        const global_int_t gix = ix + gix0;
        const global_int_t giy = iy + giy0;
        const global_int_t giz = iz + giz0;

        local_int_t currentLocalRow = i;
        global_int_t currentGlobalRow = gix + giy * gnx + giz * gnx * gny;

        A.localToGlobalMap[currentLocalRow] = currentGlobalRow;

        char numberOfNonzerosInRow = 0;
        double* currentValuePointer = matrixValues[currentLocalRow];
        global_int_t* currentIndexPointerG = mtxIndG[currentLocalRow];
        global_int_t curcol;
        double* diagonalPointer = nullptr;
        // Go through all the neighbors around a 3D point to decide
        //  which one is a halo and which one is local to the rank
        for (int k = 0; k < 27; k++)
        {
            // Neibor global Ids
            long long int cgix = gix + tid2indCpu[k][0];
            long long int cgiy = giy + tid2indCpu[k][1];
            long long int cgiz = giz + tid2indCpu[k][2];

            // These used when the point is local to the rank
            local_int_t zi = (cgiz) % nz;
            local_int_t yi = (cgiy) % ny;
            local_int_t xi = (cgix) % nx;
            // local column Id
            local_int_t lcol = zi * ny * nx + yi * nx + xi;

            // Is the global 3D point inside the global problem?
            int ok = cgiz > -1 && cgiz < gnz && cgiy > -1 && cgiy < gny && cgix > -1 && cgix < gnx;

            if (ok /*Yes this a valid point globally*/)
            {
                *currentIndexPointerG++ = cgix + cgiy * gnx + cgiz * gnx * gny;
                ;
                if (k == 13)
                {
                    *currentValuePointer = 26.0;
                    diagonalPointer = currentValuePointer;
                }
                else
                {
                    *currentValuePointer = -1.0;
                }

                // Rank Id in the global domain
                int ipz = cgiz / nz;
                int ipy = cgiy / ny;
                int ipx = cgix / nx;

                // For GPUCPU exec mode, when the CPU and GPU have diff dims in a direction,
                //  we need to find the point rank manually, not based on its local dimension
                //  but based on its physical location to the local problem
                //  Note the halo size is always 1
                if (A.geom->different_dim == Z)
                {
                    long long int local = cgiz - giz0;
                    if (local >= 0 && local < nz)
                        ipz = A.geom->ipz;
                    else if (local < 0)
                        ipz = A.geom->ipz - 1;
                    else if (local >= nz)
                        ipz = A.geom->ipz + 1;
                }
                else if (A.geom->different_dim == Y)
                {
                    long long int local = cgiy - giy0;
                    if (local >= 0 && local < ny)
                        ipy = A.geom->ipy;
                    else if (local < 0)
                        ipy = A.geom->ipy - 1;
                    else if (local >= ny)
                        ipy = A.geom->ipy + 1;
                }
                else if (A.geom->different_dim == X)
                {
                    long long int local = cgix - gix0;
                    if (local >= 0 && local < nx)
                        ipx = A.geom->ipx;
                    else if (local < 0)
                        ipx = A.geom->ipx - 1;
                    else if (local >= nx)
                        ipx = A.geom->ipx + 1;
                }

                // Now, after find the point rank from the location
                //  in the 3D grid (ranks domain NPXxNPYxNPZ)
                int col_rank = ipx + ipy * npx + ipz * npy * npx;

                // The neighbor point rank is diff than the current point rank
                if (A.geom->logical_rank != col_rank)
                {
                    if (global_steps == 2)
                        rankToId_h[col_rank + 1] = 1; // To find its sequential Id (will be prefix summed later)
                    ext_nnz++;
                }

                currentValuePointer++;
                numberOfNonzerosInRow++;
            }
        }

        matrixDiagonal[currentLocalRow] = diagonalPointer;
        nonzerosInRow[currentLocalRow] = numberOfNonzerosInRow;
        localNumberOfNonzeros += numberOfNonzerosInRow;
        if (b != 0)
            bv[currentLocalRow] = 26.0 - ((double) (numberOfNonzerosInRow - 1));
        if (x != 0)
            xv[currentLocalRow] = 0.0;
        if (xexact != 0)
            xexactv[currentLocalRow] = 1.0;
    }

    // Prefixsum to RakToId
    // Map physical neighbor ranks to sequential IDs
    //  less memory consumption
    if (global_steps == 2)
    {
        PrefixsumCpu(rankToId_h + 1, A.geom->size);
        int counter = 1;
        for (int i = 1; i < A.geom->size + 1; i++)
        {
            if (rankToId_h[i] == counter)
            {
                idToRank_h[counter - 1] = i - 1;
                counter++;
            }
        }
        global_steps++;
    }

#ifdef HPCG_DETAILED_DEBUG
    HPCG_fout << "Process " << A.geom->rank << " of " << A.geom->size << " has " << localNumberOfRows << " rows."
              << endl
              << "Process " << A.geom->rank << " of " << A.geom->size << " has " << localNumberOfNonzeros
              << " nonzeros." << endl;
#endif

    global_int_t totalNumberOfNonzeros = 27LL * ((gnx - 2LL) * (gny - 2LL) * (gnz - 2LL))
        + 18LL
            * (2LL * ((gnx - 2LL) * (gny - 2LL)) + 2LL * ((gnx - 2LL) * (gnz - 2LL))
                + 2LL * ((gny - 2LL) * (gnz - 2LL)))
        + 12LL * (4LL * (gnx - 2LL) + 4LL * (gny - 2LL) + 4LL * (gnz - 2LL)) + 8LL * 8LL;

    // If this assert fails, it most likely means that the global_int_t is set to int and should be set to long long
    // This assert is usually the first to fail as problem size increases beyond the 32-bit integer range.
    assert(totalNumberOfNonzeros
        > 0); // Throw an exception of the number of nonzeros is less than zero (can happen if int overflow)

    A.title = 0;
    A.totalNumberOfRows = totalNumberOfRows;
    A.totalNumberOfNonzeros = totalNumberOfNonzeros;
    A.localNumberOfRows = localNumberOfRows;
    A.localNumberOfColumns = localNumberOfRows;
    A.localNumberOfNonzeros = localNumberOfNonzeros;
    A.nonzerosInRow = nonzerosInRow;
    A.mtxIndG = mtxIndG;
    A.mtxIndL = mtxIndL;
    A.matrixValues = matrixValues;
    A.matrixDiagonal = matrixDiagonal;
    A.extNnz = ext_nnz;

    return;
}
#endif // USE_GRACE

void GenerateProblem(SparseMatrix& A, Vector* b, Vector* x, Vector* xexact)
{
    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
        GenerateProblem_Gpu(A, b, x, xexact);
#endif
    }
    else
    {
#ifdef USE_GRACE
        GenerateProblem_Cpu(A, b, x, xexact);
#endif
    }
}