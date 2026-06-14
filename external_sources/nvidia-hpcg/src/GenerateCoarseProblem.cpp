
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

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif

#include "GenerateCoarseProblem.hpp"
#include "GenerateGeometry.hpp"
#include "GenerateProblem.hpp"
#include "SetupHalo.hpp"
#include <cassert>
#ifdef USE_CUDA
#include "Cuda.hpp"
#endif

#ifndef HPCG_NO_MPI
// Used to find ranks for CPU and GPU programs
extern int global_total_ranks;
extern int* physical_rank_dims;
#endif

/*!
  Routine to construct a prolongation/restriction operator for a given fine grid matrix
  solution (as computed by a direct solver).

  @param[inout]  Af - The known system matrix, on output its coarse operator, fine-to-coarse operator and auxiliary
  vectors will be defined.

  Note that the matrix Af is considered const because the attributes we are modifying are declared as mutable.

*/

void GenerateCoarseProblem(const SparseMatrix& Af)
{
    // Make local copies of geometry information.  Use global_int_t since the RHS products in the calculations
    // below may result in global range values.
    global_int_t nxf = Af.geom->nx;
    global_int_t nyf = Af.geom->ny;
    global_int_t nzf = Af.geom->nz;

    local_int_t nxc, nyc, nzc; // Coarse nx, ny, nz
    assert(nxf % 2 == 0);
    assert(nyf % 2 == 0);
    assert(nzf % 2 == 0); // Need fine grid dimensions to be divisible by 2
    nxc = nxf / 2;
    nyc = nyf / 2;
    nzc = nzf / 2;
    local_int_t* f2cOperator = new local_int_t[Af.localNumberOfRows];

    local_int_t localNumberOfRows = nxc * nyc * nzc; // This is the size of our subblock
    // If this assert fails, it most likely means that the local_int_t is set to int and should be set to long long
    assert(localNumberOfRows
        > 0); // Throw an exception of the number of rows is less than zero (can happen if "int" overflows)

    for (int i = 0; i < 3 * global_total_ranks; i++)
        physical_rank_dims[i] = physical_rank_dims[i] / 2;

    // Construct the geometry and linear system
    Geometry* geomc = new Geometry;
    GenerateGeometry(Af.geom->size, Af.geom->rank, Af.geom->numThreads, nxc, nyc, nzc, Af.geom->npx, Af.geom->npy,
        Af.geom->npz, Af.geom->different_dim, geomc);
    Vector* rc = new Vector;
    Vector* xc = new Vector;
    Vector* Axf = new Vector;
    MGData* mgData = new MGData;
    if (Af.rankType == GPU)
    {
        SparseMatrix* Ac = Af.Ac;
        Ac->rankType = GPU;
        InitializeSparseMatrix(*Ac, geomc);
        GenerateProblem(*Ac, 0, 0, 0);
        SetupHalo(*Ac);
        InitializeVector(*rc, Ac->localNumberOfRows, Ac->rankType);
        InitializeVector(*xc, Ac->localNumberOfColumns, Ac->rankType);
        InitializeVector(*Axf, Af.localNumberOfColumns, Ac->rankType);
#ifdef USE_CUDA
        CHECK_CUDART(cudaMemcpy(f2cOperator, Af.gpuAux.f2c, sizeof(local_int_t) * localNumberOfRows, cudaMemcpyDeviceToHost));
#endif
    }
    else
    {
        SparseMatrix* Ac = new SparseMatrix;
        InitializeSparseMatrix(*Ac, geomc);
        Ac->rankType = CPU;
        (*Ac).Ac = 0;
        GenerateProblem(*Ac, 0, 0, 0);
        SetupHalo(*Ac);
        InitializeVector(*rc, Ac->localNumberOfRows, Ac->rankType);
        InitializeVector(*xc, Ac->localNumberOfColumns, Ac->rankType);
        InitializeVector(*Axf, Af.localNumberOfColumns, Ac->rankType);
        Af.Ac = Ac;

        // Use a parallel loop to do initial assignment:
        // distributes the physical placement of arrays of pointers across the memory system
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
        for (local_int_t i = 0; i < localNumberOfRows; ++i)
        {
            f2cOperator[i] = 0;
        }

#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
        for(local_int_t i = 0; i < nzc * nyc * nxc; i++)
        {
            local_int_t izc = (i / (nxc * nyc));
            local_int_t iyc = (i - izc * nxc * nyc) / nxc;
            local_int_t ixc = i - (izc * nyc + iyc) * nxc;

            local_int_t izf = 2 * izc;
            local_int_t iyf = 2 * iyc;
            local_int_t ixf = 2 * ixc;

            local_int_t currentCoarseRow = izc * nxc * nyc + iyc * nxc + ixc;
            local_int_t currentFineRow = izf * nxf * nyf + iyf * nxf + ixf;
            f2cOperator[currentCoarseRow] = currentFineRow;
        }
    }
    InitializeMGData(f2cOperator, rc, xc, Axf, *mgData);
    Af.mgData = mgData;

    return;
}

