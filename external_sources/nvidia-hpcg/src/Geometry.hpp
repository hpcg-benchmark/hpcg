
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
 @file Geometry.hpp

 HPCG data structure for problem geometry
 */

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

/*!
  This defines the type for integers that have local subdomain dimension.

  Define as "long long" when local problem dimension is > 2^31
*/
// #define INDEX_64

#ifndef INDEX_64
typedef int local_int_t;
#else
typedef long long local_int_t;
#endif

/*!
  This defines the type for integers that have global dimension

  Define as "long long" when global problem dimension is > 2^31
*/
#ifdef HPCG_NO_LONG_LONG
typedef int global_int_t;
#else
typedef long long global_int_t;
#endif

#define HPCG_MAX_ROW_LEN 27

// Enums
typedef enum
{
    X = 0,
    Y = 1,
    Z = 2,
    NONE = 3
} dim_3d_t;
typedef enum
{
    MPI_CPU,
    MPI_CUDA_AWARE,
    MPI_GPU_All2allv,
    MPI_CPU_All2allv,
    NCCL /*GPUONLY*/
} p2p_comm_mode_t;
typedef enum
{
    CPU,
    GPU
} rank_type_t;
typedef enum
{
    GPUONLY = 0,
    CPUONLY = 1,
    GPUCPU = 2
} exec_mode_t;
typedef enum
{
    GPU_RATIO = 0 /*NX, NY, NZ are local to GPU and g2c is a ratio*/,
    GPU_ABS = 1 /*NX, NY, NZ are local to GPU and g2c is absolute dimension size*/,
    GPU_CPU_RATIO = 2 /*NX, NY, NZ are local to GPU+CPU and g2c is ratio*/,
    GPU_CPU_ABS = 3 /*NX, NY, NZ are local to GPU+CPU and g2c is absolute dimension size*/
} local_problem_def_t;

// This macro should be defined if the global_int_t is not long long
// in order to stop complaints from non-C++11 compliant compilers.
// #define HPCG_NO_LONG_LONG

/*!
  This is a data structure to contain all processor geometry information
*/
struct Geometry_STRUCT
{
    int size;         //!< Number of MPI processes
    int rank;         //!< This process' rank in the range [0 to size - 1]
    int logical_rank; //!< For hetrogeneous setup,
    int numThreads;   //!< This process' number of threads
    local_int_t nx;   //!< Number of x-direction grid points for each local subdomain
    local_int_t ny;   //!< Number of y-direction grid points for each local subdomain
    local_int_t nz;   //!< Number of z-direction grid points for each local subdomain
    int npx;          //!< Number of processors in x-direction
    int npy;          //!< Number of processors in y-direction
    int npz;          //!< Number of processors in z-direction
    int pz;           //!< partition ID of z-dimension process that starts the second region of nz values
    int npartz;       //!< Number of partitions with varying nz values
    int* partz_ids;   //!< Array of partition ids of processor in z-direction where new value of nz starts (valid values
                      //!< are 1 to npz)
    local_int_t* partz_nz; //!< Array of length npartz containing the nz values for each partition
    int ipx;               //!< Current rank's x location in the npx by npy by npz processor grid
    int ipy;               //!< Current rank's y location in the npx by npy by npz processor grid
    int ipz;               //!< Current rank's z location in the npx by npy by npz processor grid
    global_int_t gnx;      //!< Global number of x-direction grid points
    global_int_t gny;      //!< Global number of y-direction grid points
    global_int_t gnz;      //!< Global number of z-direction grid points
    global_int_t gix0;     //!< Base global x index for this rank in the npx by npy by npz processor grid
    global_int_t giy0;     //!< Base global y index for this rank in the npx by npy by npz processor grid
    global_int_t giz0;     //!< Base global z index for this rank in the npx by npy by npz processor grid

    dim_3d_t different_dim; //!< The dimension that the GPU and CPU rank are partitioned along
    int previous_neighbor_dim;
    int next_neighbor_dim;
};
typedef struct Geometry_STRUCT Geometry;

/*!
  Returns the rank of the MPI process that is assigned the global row index
  given as the input argument.

  @param[in] geom  The description of the problem's geometry.
  @param[in] index The global row index

  @return Returns the MPI rank of the process assigned the row
*/
inline int ComputeRankOfMatrixRow(const Geometry& geom, global_int_t index)
{
    global_int_t gnx = geom.gnx;
    global_int_t gny = geom.gny;

    global_int_t iz = index / (gny * gnx);
    global_int_t iy = (index - iz * gny * gnx) / gnx;
    global_int_t ix = index % gnx;
    // We now permit varying values for nz for any nx-by-ny plane of MPI processes.
    // npartz is the number of different groups of nx-by-ny groups of processes.
    // partz_ids is an array of length npartz where each value indicates the z process of the last process in the ith
    // nx-by-ny group. partz_nz is an array of length npartz containing the value of nz for the ith group.

    //        With no variation, npartz = 1, partz_ids[0] = npz, partz_nz[0] = nz

    int ipz = 0;
    int ipartz_ids = 0;
    for (int i = 0; i < geom.npartz; ++i)
    {
        int ipart_nz = geom.partz_nz[i];
        ipartz_ids = geom.partz_ids[i] - ipartz_ids;
        if (iz <= ipart_nz * ipartz_ids)
        {
            ipz += iz / ipart_nz;
            break;
        }
        else
        {
            ipz += ipartz_ids;
            iz -= ipart_nz * ipartz_ids;
        }
    }
    //  global_int_t ipz = iz/geom.nz;
    int ipy = iy / geom.ny;
    int ipx = ix / geom.nx;
    int rank = ipx + ipy * geom.npx + ipz * geom.npy * geom.npx;
    return rank;
}

/*!
 Destructor for geometry data.

 @param[inout] data the geometry data structure whose storage is deallocated
 */
inline void DeleteGeometry(Geometry& geom)
{

    // Not used anymore
    // if(geom.partz_nz != 0)
    //   delete [] geom.partz_nz;

    // if(geom.partz_ids != 0)
    //   delete [] geom.partz_ids;

    return;
}
#endif // GEOMETRY_HPP
