
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
 @file GenerateGeometry.cpp

 HPCG routine
 */

#include <cassert>
#include <cmath>
#include <cstdlib>

#include "ComputeOptimalShapeXYZ.hpp"
#include "GenerateGeometry.hpp"

#include <cstdio>

#ifdef HPCG_DEBUG
#include "hpcg.hpp"
#include <fstream>
using std::endl;

#endif

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

#ifndef HPCG_NO_MPI
// Used to find ranks for CPU and GPU programs
extern int global_total_ranks;
extern int* physical_rank_dims;
extern int* logical_rank_to_phys;
#endif

/*!
  Computes the factorization of the total number of processes into a
  3-dimensional process grid that is as close as possible to a cube. The
  quality of the factorization depends on the prime number structure of the
  total number of processes. It then stores this decompostion together with the
  parallel parameters of the run in the geometry data structure.

  @param[in]  size total number of MPI processes
  @param[in]  rank this process' rank among other MPI processes
  @param[in]  numThreads number of OpenMP threads in this process
  @param[in]  nx, ny, nz number of grid points for each local block in the x, y, and z dimensions, respectively
  @param[out] geom data structure that will store the above parameters and the factoring of total number of processes
  into three dimensions
*/

// Level 0 Generation, we need to decide nx, ny, nz based on
// G2C ratio and npx, npy, npz
//  Remap rank IDs to logical IDs to enforce 3D shape correctness when exec_mode is GPUCPU
void GenerateGeometry(HPCG_Params& params, Geometry* geom)
{
    int size = params.comm_size, rank = params.comm_rank; // Number of MPI processes, My process ID
    int nx = params.nx, ny = params.ny, nz = params.nz;
    int npx = params.npx, npy = params.npy, npz = params.npz;

    // If npx. npy, and npz are not provided by user
    // find the optimal shape
    if (npx * npy * npz <= 0 || npx * npy * npz > size)
        ComputeOptimalShapeXYZ(size, npx, npy, npz);

    // When search_for_same0 is true, finds the next rank that is the same as local
    //  problem size as rank 0. When false, finds the ranks that are not the same as rank 0
    auto loop_over_ranks = [](int index, int lp, bool search_for_same0) -> int
    {
        for (int p = index; p < global_total_ranks; p++)
        {
            int nnpx = physical_rank_dims[3 * p];
            int nnpy = physical_rank_dims[3 * p + 1];
            int nnpz = physical_rank_dims[3 * p + 2];
            bool same_zero = false;
            if (nnpx == physical_rank_dims[0] && nnpy == physical_rank_dims[1] && nnpz == physical_rank_dims[2])
                same_zero = true;

            if (same_zero == search_for_same0)
            {
                logical_rank_to_phys[lp] = p;
                index = p + 1;
                break;
            }
        }
        return index;
    };

    // Here decide and broadcast nx, ny, nz
    // 1 Check for GPU and CPU execution modes
    auto user_diff_dim = NONE;
    if (params.exec_mode == GPUCPU)
    {
        // User defined diff direction between GPU and CPU
        // If user decides that nz should be diff between GPU and CPU
        //  and NPZ is even --> Decide GPU and CPU local size based on
        //  local_problem_def and g2c
        if (params.diff_dim == Z && (npz & 1) == 0)
        {
            user_diff_dim = Z;
            if (params.local_problem_def == GPU_RATIO)
            {
                if (params.rank_type == CPU)
                    nz = nz / params.g2c;
            }
            else if (params.local_problem_def == GPU_ABS)
            {
                if (params.rank_type == CPU)
                    nz = params.g2c;
            }
            else if (params.local_problem_def == GPU_CPU_RATIO)
            {
                if (params.rank_type == CPU)
                    nz = nz / params.g2c;
                if (params.rank_type == GPU)
                    nz = nz - (nz / params.g2c);
            }
            else
            { /*GPU_CPU_ABS*/
                if (params.rank_type == CPU)
                    nz = params.g2c;
                if (params.rank_type == GPU)
                    nz = nz - params.g2c;
            }
        }
        // If user decides that ny should be diff between GPU and CPU
        //  and NPY is even --> Decide GPU and CPU local size based on
        //  local_problem_def and g2c
        else if (params.diff_dim == Y && (npy & 1) == 0)
        {
            user_diff_dim = Y;
            if (params.local_problem_def == GPU_RATIO)
            {
                if (params.rank_type == CPU)
                    ny = ny / params.g2c;
            }
            else if (params.local_problem_def == GPU_ABS)
            {
                if (params.rank_type == CPU)
                    ny = params.g2c;
            }
            else if (params.local_problem_def == GPU_CPU_RATIO)
            {
                if (params.rank_type == CPU)
                    ny = ny / params.g2c;
                if (params.rank_type == GPU)
                    ny = ny - (ny / params.g2c);
            }
            else
            { /*GPU_CPU_ABS*/
                if (params.rank_type == CPU)
                    ny = params.g2c;
                if (params.rank_type == GPU)
                    ny = ny - params.g2c;
            }
        }
        // If user decides that nx should be diff between GPU and CPU
        //  and NPX is even --> Decide GPU and CPU local size based on
        //  local_problem_def and g2c
        else if (params.diff_dim == X && (npx & 1) == 0)
        {
            user_diff_dim = X;
            if (params.local_problem_def == GPU_RATIO)
            {
                if (params.rank_type == CPU)
                    nx = nx / params.g2c;
            }
            else if (params.local_problem_def == GPU_ABS)
            {
                if (params.rank_type == CPU)
                    nx = params.g2c;
            }
            else if (params.local_problem_def == GPU_CPU_RATIO)
            {
                if (params.rank_type == CPU)
                    nx = nx / params.g2c;
                if (params.rank_type == GPU)
                    nx = nx - (nx / params.g2c);
            }
            else
            { /*GPU_CPU_ABS*/
                if (params.rank_type == CPU)
                    nx = params.g2c;
                if (params.rank_type == GPU)
                    nx = nx - params.g2c;
            }
        }
        // Automatic partition direction
        // When user does not specify the diff dimension
        if (user_diff_dim == NONE)
        { // Did not succeed with user choice
            if ((npz & 1) == 0)
            {
                if (params.local_problem_def == GPU_RATIO)
                {
                    if (params.rank_type == CPU)
                        nz = nz / params.g2c;
                }
                else if (params.local_problem_def == GPU_ABS)
                {
                    if (params.rank_type == CPU)
                        nz = params.g2c;
                }
                else if (params.local_problem_def == GPU_CPU_RATIO)
                {
                    if (params.rank_type == CPU)
                        nz = nz / params.g2c;
                    if (params.rank_type == GPU)
                        nz = nz - (nz / params.g2c);
                }
                else
                { /*GPU_CPU_ABS*/
                    if (params.rank_type == CPU)
                        nz = params.g2c;
                    if (params.rank_type == GPU)
                        nz = nz - params.g2c;
                }
            }
            else if ((npy & 1) == 0)
            {
                if (params.local_problem_def == GPU_RATIO)
                {
                    if (params.rank_type == CPU)
                        ny = ny / params.g2c;
                }
                else if (params.local_problem_def == GPU_ABS)
                {
                    if (params.rank_type == CPU)
                        ny = params.g2c;
                }
                else if (params.local_problem_def == GPU_CPU_RATIO)
                {
                    if (params.rank_type == CPU)
                        ny = ny / params.g2c;
                    if (params.rank_type == GPU)
                        ny = ny - (ny / params.g2c);
                }
                else
                { /*GPU_CPU_ABS*/
                    if (params.rank_type == CPU)
                        ny = params.g2c;
                    if (params.rank_type == GPU)
                        ny = ny - params.g2c;
                }
            }
            else if ((npx & 1) == 0)
            {
                if (params.local_problem_def == GPU_RATIO)
                {
                    if (params.rank_type == CPU)
                        nx = nx / params.g2c;
                }
                else if (params.local_problem_def == GPU_ABS)
                {
                    if (params.rank_type == CPU)
                        nx = params.g2c;
                }
                else if (params.local_problem_def == GPU_CPU_RATIO)
                {
                    if (params.rank_type == CPU)
                        nx = nx / params.g2c;
                    if (params.rank_type == GPU)
                        nx = nx - (nx / params.g2c);
                }
                else
                { /*GPU_CPU_ABS*/
                    if (params.rank_type == CPU)
                        nx = params.g2c;
                    if (params.rank_type == GPU)
                        nx = nx - params.g2c;
                }
            }
        }
    }

    // Now let us exchange dimensions
    int sendBuf[] = {nx, ny, nz};
#ifndef HPCG_NO_MPI
    MPI_Allgather(sendBuf, 3, MPI_INT, physical_rank_dims, 3, MPI_INT, MPI_COMM_WORLD);
#endif

    // My logical rank Id
    int logical_rank;
    // last physical position for the rank that has the same size as 0
    int same_as_0_position = 0;
    // last physical position for the rank that does not have the same size as 0
    int not_same_as_0_position = 0;
    auto different_dim = NONE;

    bool all_same = true;
    int num_ranks_same = 1;
    int num_ranks_not_same = 0;
    int x0 = physical_rank_dims[0];
    int y0 = physical_rank_dims[1];
    int z0 = physical_rank_dims[2];
    for (int p = 1; p < global_total_ranks; p++)
    {
        int x = physical_rank_dims[3 * p];
        int y = physical_rank_dims[3 * p + 1];
        int z = physical_rank_dims[3 * p + 2];
        if (x != x0 || y != y0 || z != z0)
            num_ranks_not_same++;
        else
            num_ranks_same++;
    }

    if (num_ranks_not_same > 0)
        all_same = false;

    if (!all_same)
    {
        // try twice: user-based, automatic
        for (int i = 0; i < 2; i++)
        {
            bool z_condition = (i == 0) ? user_diff_dim == Z && (npz & 1) == 0 : (npz & 1) == 0;
            bool y_condition = (i == 0) ? user_diff_dim == Y && (npy & 1) == 0 : (npy & 1) == 0;
            bool x_condition = (i == 0) ? user_diff_dim == X && (npx & 1) == 0 : (npx & 1) == 0;
            // Let us start with Z
            if (z_condition)
            { // Z is even
                different_dim = Z;
                bool x_same = true;
                bool y_same = true;
                for (int p = 1; p < global_total_ranks; p++)
                {
                    int x = physical_rank_dims[3 * p];
                    int y = physical_rank_dims[3 * p + 1];
                    assert(x == x0 && y == y0);
                }
            }
            else if (y_condition)
            { // Y is even
                different_dim = Y;
                bool x_same = true;
                bool z_same = true;
                for (int p = 1; p < global_total_ranks; p++)
                {
                    int x = physical_rank_dims[3 * p];
                    int z = physical_rank_dims[3 * p + 2];
                    assert(x == x0 && z == z0);
                }
            }
            else if (x_condition)
            {
                different_dim = X;
                bool y_same = true;
                bool z_same = true;
                for (int p = 1; p < global_total_ranks; p++)
                {
                    int y = physical_rank_dims[3 * p + 1];
                    int z = physical_rank_dims[3 * p + 2];
                    assert(z == z0 && y == y0);
                }
            }

            if (z_condition || y_condition || x_condition)
                break;
        }
    }

    // When exec_mode is GPUCPU, GPU and CPU ranks can have different dims. Therefore,
    // we must rearrange the ranks such that the 3D shape is correct.
    int same_rank_counter = 0;
    if (different_dim != NONE)
    {
        for (int iz = 0; iz < npz; iz++)
            for (int iy = 0; iy < npy; iy++)
                for (int ix = 0; ix < npx; ix++)
                {
                    int logical_position = iz * npy * npx + iy * npx + ix;

                    // Different dim is Z
                    // The first NPXxNPY are GPUs, then the next NPXxNPY is CPUs, and so on
                    if (different_dim == Z)
                    {
                        if ((iz & 1) == 0 && same_rank_counter < num_ranks_same)
                        { // same as 0
                            same_as_0_position = loop_over_ranks(same_as_0_position, logical_position, true);
                            same_rank_counter++;
                        }
                        else
                        { // Not same as 0
                            not_same_as_0_position = loop_over_ranks(not_same_as_0_position, logical_position, false);
                        }
                    }
                    // Different dim is Y
                    // The first NPXxNPZ are GPUs, then the next NPXxNPZ is CPUs, and so on
                    else if (different_dim == Y)
                    {
                        if ((iy & 1) == 0 && same_rank_counter < num_ranks_same)
                        { // same as 0
                            same_as_0_position = loop_over_ranks(same_as_0_position, logical_position, true);
                            same_rank_counter++;
                        }
                        else
                        { // Not same as 0
                            not_same_as_0_position = loop_over_ranks(not_same_as_0_position, logical_position, false);
                        }
                    }
                    // Different dim is X
                    // The first NPYxNPZ are GPUs, then the next NPYxNPZ is CPUs, and so on
                    else if (different_dim == X)
                    {
                        if ((ix & 1) == 0 && same_rank_counter < num_ranks_same)
                        { // same as 0
                            same_as_0_position = loop_over_ranks(same_as_0_position, logical_position, true);
                            same_rank_counter++;
                        }
                        else
                        { // Not same as 0
                            not_same_as_0_position = loop_over_ranks(not_same_as_0_position, logical_position, false);
                        }
                    }
                }
    }
    else
    {
        // Keep rank Ids the same if all ranks have the same problem size
        for (int p = 0; p < global_total_ranks; p++)
            logical_rank_to_phys[p] = p;
    }

    for (int p = 0; p < global_total_ranks; p++)
    {
        if (rank == logical_rank_to_phys[p])
        {
            logical_rank = p;
        }
    }

    // Now compute this process's indices in the 3D cube
    int ipz = logical_rank / (npx * npy);
    int ipy = (logical_rank - ipz * npx * npy) / npx;
    int ipx = logical_rank % npx;

#ifdef HPCG_DEBUG
    if (rank == 0)
        HPCG_fout << "size = " << size << endl
                  << "nx  = " << nx << endl
                  << "ny  = " << ny << endl
                  << "nz  = " << nz << endl
                  << "npx = " << npx << endl
                  << "npy = " << npy << endl
                  << "npz = " << npz << endl;

    HPCG_fout << "For rank = " << rank << endl
              << "ipx = " << ipx << endl
              << "ipy = " << ipy << endl
              << "ipz = " << ipz << endl;

    assert(size >= npx * npy * npz);
#endif
    geom->size = size;
    geom->rank = rank;
    geom->logical_rank = logical_rank;
    geom->different_dim = different_dim;
    geom->numThreads = params.numThreads;
    geom->nx = nx;
    geom->ny = ny;
    geom->nz = nz;
    geom->npx = npx;
    geom->npy = npy;
    geom->npz = npz;
    geom->ipx = ipx;
    geom->ipy = ipy;
    geom->ipz = ipz;

    // These values should be defined to take into account changes in nx, ny, nz values
    // due to variable local grid sizes
    global_int_t gnx = 0;
    global_int_t gny = 0;
    global_int_t gnz = 0;

    // Find the global NX. NY, and NZ
    //  For diff dims, accumulate sequentially
    //  For similar dims, just multiply rank 3D location by the local dim
    if (different_dim == X)
        for (int i = 0; i < npx; i++)
        {
            int r = ipz * npx * npy + ipy * npx + i;
            int p = logical_rank_to_phys[r];
            gnx += physical_rank_dims[p * 3];
        }
    else
        gnx = npx * nx;

    if (different_dim == Y)
        for (int i = 0; i < npy; i++)
        {
            int r = ipz * npx * npy + i * npx + ipx;
            int p = logical_rank_to_phys[r];
            gny += physical_rank_dims[p * 3 + 1];
        }
    else
        gny = npy * ny;

    if (different_dim == Z)
        for (int i = 0; i < npz; i++)
        {
            int r = i * npx * npy + ipy * npx + ipx;
            int p = logical_rank_to_phys[r];
            gnz += physical_rank_dims[p * 3 + 2];
        }
    else
        gnz = npz * nz;

    // Here, we find the initial global indices (gix0, giy0, and giz0)
    // for each rank based on its 3d location in the grid
    // Also, for the diff dim find the previous and next neighbor IDs
    // Notice, on the diff dims the previous and next neighbors have
    // the different dimension!
    int prev_n = 0;
    int next_n = 0;
    global_int_t giz0 = 0;
    global_int_t gix0 = 0;
    global_int_t giy0 = 0;
    if (different_dim == X)
    {
        for (int i = 0; i < ipx; i++)
        {
            int r = ipz * npx * npy + ipy * npx + i;
            int p = logical_rank_to_phys[r];
            gix0 += physical_rank_dims[p * 3];
            if (i == ipx - 1)
            {
                prev_n = physical_rank_dims[p * 3];
            }
        }
        if (ipx + 1 < npx)
        {
            int r = ipz * npx * npy + ipy * npx + (ipx + 1);
            int p = logical_rank_to_phys[r];
            next_n = physical_rank_dims[p * 3];
        }
    }
    else
        gix0 = ipx * nx;

    if (different_dim == Y)
    {
        for (int i = 0; i < ipy; i++)
        {
            int r = ipz * npx * npy + i * npx + ipx;
            int p = logical_rank_to_phys[r];
            giy0 += physical_rank_dims[p * 3 + 1];
            if (i == ipy - 1)
            {
                prev_n = physical_rank_dims[p * 3 + 1];
            }
        }
        if (ipy + 1 < npy)
        {
            int r = ipz * npx * npy + (ipy + 1) * npx + ipx;
            int p = logical_rank_to_phys[r];
            next_n = physical_rank_dims[p * 3 + 1];
        }
    }
    else
        giy0 = ipy * ny;

    if (different_dim == Z)
    {
        for (int i = 0; i < ipz; i++)
        {
            int r = i * npx * npy + ipy * npx + ipx;
            int p = logical_rank_to_phys[r];
            giz0 += physical_rank_dims[p * 3 + 2];
            if (i == ipz - 1)
            {
                prev_n = physical_rank_dims[p * 3 + 2];
            }
        }
        if (ipz + 1 < npz)
        {
            int r = (ipz + 1) * npx * npy + ipy * npx + ipx;
            int p = logical_rank_to_phys[r];
            next_n = physical_rank_dims[p * 3 + 2];
        }
    }
    else
        giz0 = ipz * nz;

    // Keep these values for later
    geom->gnx = gnx;
    geom->gny = gny;
    geom->gnz = gnz;
    geom->gix0 = gix0;
    geom->giy0 = giy0;
    geom->giz0 = giz0;
    geom->previous_neighbor_dim = prev_n;
    geom->next_neighbor_dim = next_n;

    return;
}

// Simpler generateion for next/coarse levels
// Do not need to find nx, ny, nz for CPU and GPU based on parameters
// Do not need to find logical rank IDs
void GenerateGeometry(int size, int rank, int numThreads, local_int_t nx, local_int_t ny, local_int_t nz, int npx,
    int npy, int npz, dim_3d_t different_dim, Geometry* geom)
{

    // My logical rank Id
    int logical_rank;
    for (int p = 0; p < global_total_ranks; p++)
    {
        if (rank == logical_rank_to_phys[p])
        {
            logical_rank = p;
        }
    }

    // Now compute this process's indices in the 3D cube
    int ipz = logical_rank / (npx * npy);
    int ipy = (logical_rank - ipz * npx * npy) / npx;
    int ipx = logical_rank % npx;

#ifdef HPCG_DEBUG
    if (rank == 0)
        HPCG_fout << "size = " << size << endl
                  << "nx  = " << nx << endl
                  << "ny  = " << ny << endl
                  << "nz  = " << nz << endl
                  << "npx = " << npx << endl
                  << "npy = " << npy << endl
                  << "npz = " << npz << endl;

    HPCG_fout << "For rank = " << rank << endl
              << "ipx = " << ipx << endl
              << "ipy = " << ipy << endl
              << "ipz = " << ipz << endl;

    assert(size >= npx * npy * npz);
#endif
    geom->size = size;
    geom->rank = rank;
    geom->logical_rank = logical_rank;
    geom->different_dim = different_dim;
    geom->numThreads = numThreads;
    geom->nx = nx;
    geom->ny = ny;
    geom->nz = nz;
    geom->npx = npx;
    geom->npy = npy;
    geom->npz = npz;
    geom->ipx = ipx;
    geom->ipy = ipy;
    geom->ipz = ipz;

    // Find the global NX. NY, and NZ
    //  For diff dims, accumulate sequentially
    //  For similar dims, just multiply rank 3D location by the local dim
    global_int_t gnx = 0;
    global_int_t gny = 0;
    global_int_t gnz = 0;
    if (different_dim == X)
        for (int i = 0; i < npx; i++)
        {
            int r = ipz * npx * npy + ipy * npx + i;
            int p = logical_rank_to_phys[r];
            gnx += physical_rank_dims[p * 3];
        }
    else
        gnx = npx * nx;

    if (different_dim == Y)
        for (int i = 0; i < npy; i++)
        {
            int r = ipz * npx * npy + i * npx + ipx;
            int p = logical_rank_to_phys[r];
            gny += physical_rank_dims[p * 3 + 1];
        }
    else
        gny = npy * ny;

    if (different_dim == Z)
        for (int i = 0; i < npz; i++)
        {
            int r = i * npx * npy + ipy * npx + ipx;
            int p = logical_rank_to_phys[r];
            gnz += physical_rank_dims[p * 3 + 2];
        }
    else
        gnz = npz * nz;

    // Here, we find the initial global indices (gix0, giy0, and giz0)
    // for each rank based on its 3d location in the grid
    // Also, for the diff dim find the previous and next neighbor IDs
    // Notice, on the diff dims the previous and next neighbors have
    // the different dimension!
    int prev_n = 0;
    int next_n = 0;
    global_int_t giz0 = 0;
    global_int_t gix0 = 0;
    global_int_t giy0 = 0;
    if (different_dim == X)
    {
        for (int i = 0; i < ipx; i++)
        {
            int r = ipz * npx * npy + ipy * npx + i;
            int p = logical_rank_to_phys[r];
            gix0 += physical_rank_dims[p * 3];
            if (i == ipx - 1)
            {
                prev_n = physical_rank_dims[p * 3];
            }
        }
        if (ipx + 1 < npx)
        {
            int r = ipz * npx * npy + ipy * npx + (ipx + 1);
            int p = logical_rank_to_phys[r];
            next_n = physical_rank_dims[p * 3];
        }
    }
    else
        gix0 = ipx * nx;

    if (different_dim == Y)
    {
        for (int i = 0; i < ipy; i++)
        {
            int r = ipz * npx * npy + i * npx + ipx;
            int p = logical_rank_to_phys[r];
            giy0 += physical_rank_dims[p * 3 + 1];
            if (i == ipy - 1)
            {
                prev_n = physical_rank_dims[p * 3 + 1];
            }
        }
        if (ipy + 1 < npy)
        {
            int r = ipz * npx * npy + (ipy + 1) * npx + ipx;
            int p = logical_rank_to_phys[r];
            next_n = physical_rank_dims[p * 3 + 1];
        }
    }
    else
        giy0 = ipy * ny;

    if (different_dim == Z)
    {
        for (int i = 0; i < ipz; i++)
        {
            int r = i * npx * npy + ipy * npx + ipx;
            int p = logical_rank_to_phys[r];
            giz0 += physical_rank_dims[p * 3 + 2];
            if (i == ipz - 1)
            {
                prev_n = physical_rank_dims[p * 3 + 2];
            }
        }
        if (ipz + 1 < npz)
        {
            int r = (ipz + 1) * npx * npy + ipy * npx + ipx;
            int p = logical_rank_to_phys[r];
            next_n = physical_rank_dims[p * 3 + 2];
        }
    }
    else
        giz0 = ipz * nz;

    // Keep these values for later
    geom->gnx = gnx;
    geom->gny = gny;
    geom->gnz = gnz;
    geom->gix0 = gix0;
    geom->giy0 = giy0;
    geom->giz0 = giz0;
    geom->previous_neighbor_dim = prev_n;
    geom->next_neighbor_dim = next_n;

    return;
}
