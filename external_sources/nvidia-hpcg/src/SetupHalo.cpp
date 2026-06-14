
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
 @file SetupHalo.cpp

 HPCG routine
 */

#ifndef HPCG_NO_MPI
#include <map>
#include <mpi.h>
#include <set>
#endif

#include <algorithm>

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif

#include "SetupHalo.hpp"
#include "SetupHalo_ref.hpp"

#ifdef USE_CUDA
#include "Cuda.hpp"
#include "CudaKernels.hpp"
#ifdef USE_NCCL
extern ncclComm_t Nccl_Comm;
#endif
#endif

#ifdef USE_GRACE
#include "CpuKernels.hpp"
#endif

#ifndef HPCG_NO_MPI
// Used to find ranks for CPU and GPU programs
extern int global_total_ranks;
extern int* physical_rank_dims;
extern int* logical_rank_to_phys;
extern int* rankToId_h;
extern int* idToRank_h;
extern p2p_comm_mode_t P2P_Mode;
#endif

/*!
  Prepares system matrix data structure and creates data necessary necessary
  for communication of boundary values of this process.

  @param[inout] A    The known system matrix

  @see ExchangeHalo
*/
#ifdef USE_CUDA
void SetupHalo_Gpu(SparseMatrix& A)
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

#ifndef HPCG_NO_MPI
    local_int_t localNumberOfRows = A.localNumberOfRows;

    local_int_t* send_buffer_d;
    local_int_t sendbufld
        = std::max(std::max(A.geom->nx * A.geom->ny, A.geom->nx * A.geom->nz), A.geom->ny * A.geom->nz);
    int* neighbors = new int[27];
    int* neighborsPhysical = new int[27];

    CHECK_CUDART(cudaMalloc((void**) &(send_buffer_d), 27 * sendbufld * sizeof(local_int_t)));
    local_int_t* sendLength = new local_int_t[27];

    local_int_t totalToBeSent = 0;
    int neiCount = 0;
    int numberOfExternalValues = 0;

    local_int_t* sendcounts2 = new local_int_t[27];
    local_int_t* receiveLength = new local_int_t[27];
    memset(sendcounts2, 0, sizeof(local_int_t) * (27));

    local_int_t* sendcounts_d = NULL;
    local_int_t* elementsToSendGpu;

    CHECK_CUDART(cudaMalloc(&sendcounts_d, sizeof(local_int_t) * (27)));
    CHECK_CUDART(cudaMemsetAsync(sendcounts_d, 0, sizeof(local_int_t) * (27), stream));

    // Finds elements to send and neighbors
    SetupHaloCuda(A, sendbufld, sendcounts_d, send_buffer_d, &totalToBeSent, &neiCount, neighbors, sendLength,
        &elementsToSendGpu);

    // Host copy is only needed for MPI index exchange; GPU runtime uses gpuAux.elementsToSend.
    local_int_t* elementsToSend = new local_int_t[totalToBeSent];
    double* sendBuffer = nullptr;
    if (totalToBeSent > 0)
    {
        cudaMemcpyAsync(
            elementsToSend, elementsToSendGpu, sizeof(local_int_t) * totalToBeSent, cudaMemcpyDeviceToHost, stream);

        local_int_t* sendcounts = (local_int_t*) malloc(sizeof(local_int_t) * (A.geom->size + 1));
        memset(sendcounts, 0, sizeof(local_int_t) * (A.geom->size + 1));

        local_int_t *eltsToRecv_d = NULL, *extToLocMap = NULL;

        sendcounts[0] = 0;
        for (int i = 0; i < neiCount; i++)
        {
            receiveLength[i] = sendLength[i];
            sendcounts[i + 1] = sendcounts[i] + sendLength[i];
            int neighborId = neighbors[i];
            neighborsPhysical[i] = logical_rank_to_phys[neighborId];
        }
        CHECK_CUDART(cudaMalloc(&extToLocMap, sizeof(local_int_t) * localNumberOfRows));
        CHECK_CUDART(cudaMalloc(&eltsToRecv_d, sizeof(local_int_t) * totalToBeSent));

        CHECK_CUDART(cudaMallocHost(&(sendBuffer), sizeof(double) * totalToBeSent));
        CHECK_CUDART(cudaMalloc(&(A.gpuAux.sendBuffer), sizeof(double) * totalToBeSent));

        CHECK_CUDART(cudaStreamSynchronize(stream));

#if defined(USE_NCCL) && !defined(HPCG_NO_MPI)
        // P2P=NCCL is rejected in main unless --exm=0 (GPUONLY); Nccl_Comm is never created otherwise.
        if (P2P_Mode == NCCL)
        {
#ifndef INDEX_64
            const ncclDataType_t ncclIdx = ncclInt32;
#else
            const ncclDataType_t ncclIdx = ncclInt64;
#endif
            CHECK_NCCL(ncclGroupStart());
            local_int_t* send_d = elementsToSendGpu;
            local_int_t* recv_d = eltsToRecv_d;
            for (int i = 0; i < neiCount; i++)
            {
                const int peer = neighborsPhysical[i];
                const local_int_t n_send = sendLength[i];
                CHECK_NCCL(ncclSend(send_d, n_send, ncclIdx, peer, Nccl_Comm, stream));
                send_d += n_send;
                const local_int_t n_recv = receiveLength[i];
                CHECK_NCCL(ncclRecv(recv_d, n_recv, ncclIdx, peer, Nccl_Comm, stream));
                recv_d += n_recv;
            }
            CHECK_NCCL(ncclGroupEnd());
            CHECK_CUDART(cudaStreamSynchronize(stream));
        }
        else
#endif
        {
            local_int_t* eltsToRecv = new local_int_t[totalToBeSent];

            // Exchange elements to send with neighbors
            auto INDEX_TYPE = MPI_INT;
#ifdef INDEX_64 // In src/Geometry
            INDEX_TYPE = MPI_LONG;
#endif

            MPI_Status status;
            int MPI_MY_TAG = 93;
            MPI_Request* request = new MPI_Request[neiCount];

            local_int_t* recv_ptr = eltsToRecv;
            for (int i = 0; i < neiCount; i++)
            {
                auto n_recv = sendLength[i];
                MPI_Irecv(recv_ptr, n_recv, INDEX_TYPE, neighborsPhysical[i], MPI_MY_TAG, MPI_COMM_WORLD, request + i);
                recv_ptr += n_recv;
            }

            local_int_t* elts_ptr = elementsToSend;
            for (int i = 0; i < neiCount; i++)
            {
                auto n_send = sendLength[i];
                MPI_Send(elts_ptr, n_send, INDEX_TYPE, neighborsPhysical[i], MPI_MY_TAG, MPI_COMM_WORLD);
                elts_ptr += n_send;
            }
            for (int i = 0; i < neiCount; i++)
            {
                MPI_Wait(request + i, &status);
            }
            delete[] request;

            CHECK_CUDART(cudaMemcpyAsync(
                eltsToRecv_d, eltsToRecv, sizeof(local_int_t) * (totalToBeSent), cudaMemcpyHostToDevice, stream));
            CHECK_CUDART(cudaStreamSynchronize(stream));
            delete[] eltsToRecv;
        }

        // Add the sorted indices from neighbors. For each neighbor, add its indices sequentially
        //  before the next neighbor's indices. Tje indices will be adjusted to be
        //  localNumberOfRows + its sequential location
        for (int neighborCount = 0; neighborCount < neiCount; ++neighborCount)
        {
            int neighborId = neighbors[neighborCount];
            CHECK_CUDART(cudaMemsetAsync(extToLocMap, 0, sizeof(local_int_t) * localNumberOfRows, stream));
            local_int_t str = sendcounts[neighborCount];
            local_int_t end = sendcounts[neighborCount + 1];
            ExtToLocMapCuda(localNumberOfRows, str, end, extToLocMap, eltsToRecv_d);
            ExtTolocCuda(localNumberOfRows, neighborId, A.extNnz, A.csrExtColumns, A.csrExtValues,
                A.gpuAux.ext2csrOffsets, extToLocMap, A.gpuAux.columns);
        }

        CHECK_CUDART(cudaFree(sendcounts_d));
        CHECK_CUDART(cudaFree(extToLocMap));
        CHECK_CUDART(cudaFree(eltsToRecv_d));

        // For P2P Alltoallv communication
        if (P2P_Mode == MPI_GPU_All2allv || P2P_Mode == MPI_CPU_All2allv)
        {
            int* sdispls = new int[A.geom->size];
            int* rdispls = new int[A.geom->size];
            int* scounts = new int[A.geom->size];
            int* rcounts = new int[A.geom->size];
            int tmp_s = 0, tmp_r = 0;

            if (sdispls == NULL || rdispls == NULL || scounts == NULL || rcounts == NULL)
                return;

            for (local_int_t i = 0; i < A.geom->size; i++)
            {
                scounts[i] = 0;
                rcounts[i] = 0;
                sdispls[i] = 0;
                rdispls[i] = 0;
            }

            for (local_int_t i = 0; i < neiCount; i++)
            {
                local_int_t root = neighborsPhysical[i];
                scounts[root] = sendLength[i];
                rcounts[root] = receiveLength[i];
                sdispls[root] = tmp_s;
                tmp_s += sendLength[i];
                rdispls[root] = tmp_r;
                tmp_r += receiveLength[i];
            }

            A.scounts = scounts;
            A.rcounts = rcounts;
            A.sdispls = sdispls;
            A.rdispls = rdispls;
        }
    }

    // Store contents in our matrix struct
    A.numberOfExternalValues = totalToBeSent;
    A.localNumberOfColumns = A.localNumberOfRows + A.numberOfExternalValues;
    A.numberOfSendNeighbors = neiCount;
    A.totalToBeSent = totalToBeSent;
    A.elementsToSend = elementsToSend;
    A.gpuAux.elementsToSend = elementsToSendGpu;
    A.neighbors = neighbors;
    A.neighborsPhysical = neighborsPhysical;
    A.receiveLength = receiveLength;
    A.sendLength = sendLength;
    A.sendBuffer = sendBuffer;
#endif
    return;
}
#endif

#ifdef USE_GRACE
void SetupHalo_Cpu(SparseMatrix& A)
{
    // Extract Matrix pieces
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

    local_int_t localNumberOfRows = A.localNumberOfRows;
    local_int_t* nonzerosInRow = A.nonzerosInRow;
    global_int_t** mtxIndG = A.mtxIndG;
    local_int_t** mtxIndL = A.mtxIndL;

#ifdef HPCG_NO_MPI // In the non-MPI case we simply copy global indices to local index storage
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
    for (local_int_t i = 0; i < localNumberOfRows; i++)
    {
        int cur_nnz = nonzerosInRow[i];
        for (int j = 0; j < cur_nnz; j++)
            mtxIndL[i][j] = mtxIndG[i][j];
    }

#else // Run this section if compiling for MPI

    // Scan global IDs of the nonzeros in the matrix.  Determine if the column ID matches a row ID.  If not:
    // 1) We call the ComputeRankOfMatrixRow function, which tells us the rank of the processor owning the row ID.
    //  We need to receive this value of the x vector during the halo exchange.
    // 2) We record our row ID since we know that the other processor will need this value from us, due to symmetry.
    std::map<local_int_t, std::map<global_int_t, local_int_t>> externalToLocalMap;
    local_int_t* extTemp = new local_int_t[localNumberOfRows];

    // Okay Let us git rid of the map
    local_int_t sendbufld
        = std::max(std::max(A.geom->nx * A.geom->ny, A.geom->nx * A.geom->nz), A.geom->ny * A.geom->nz);
    local_int_t* send_buffer = new local_int_t[27 * sendbufld];
    char* has_external = new char[localNumberOfRows];
    local_int_t* sendcounter = new local_int_t[27];
    for (local_int_t i = 0; i < 27; i++)
        sendcounter[i] = 0;

// Goes through all local rows, for each local point
//  find its 27 3D neighbors (including the point itself).
//  For each neibor decide if it is on a different rank (halo) or local
//  If external, add to the send buffer
//  If local, create the local matrix
#pragma omp parallel for
    for (local_int_t i = 0; i < localNumberOfRows; i++)
    {
        const local_int_t iz = (i / (nx * ny));
        const local_int_t iy = (i - iz * nx * ny) / nx;
        const local_int_t ix = i - (iz * ny + iy) * nx;
        const global_int_t gix = ix + gix0;
        const global_int_t giy = iy + giy0;
        const global_int_t giz = iz + giz0;
        global_int_t curcol;

        int nnz_c = 0;
        bool rank_set[27];
        for (int j = 0; j < 27; j++)
        {
            rank_set[j] = false;
        }
        has_external[i] = 0;
        for (int k = 0; k < 27; k++)
        {
            long long int cgix = gix + tid2indCpu[k][0];
            long long int cgiy = giy + tid2indCpu[k][1];
            long long int cgiz = giz + tid2indCpu[k][2];
            int ok = cgiz > -1 && cgiz < gnz && cgiy > -1 && cgiy < gny && cgix > -1 && cgix < gnx;
            if (ok)
            {
                int ipz = cgiz / nz;
                int ipy = cgiy / ny;
                int ipx = cgix / nx;

                // For GPUCPU exec mode, find the 3D rank coordinates.
                //  For diff dim between CPU and GPU, we cannot
                //  just divide on the local dim to find ipx/ipy/ipz
                //  We must find it manually based on neighbor 3d coordinates
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

                // Global rank Id
                int col_rank = ipx + ipy * npx + ipz * npy * npx;

                // The neighbor point rank is diff than the current point rank
                if (A.geom->logical_rank != col_rank)
                {
                    has_external[i] = 1;
                    int rankId = rankToId_h[col_rank];
                    local_int_t* p = &(sendcounter[rankId]);
                    // Add the halo point atomically to send_buffer
                    // For all the cols in a row that has the same rank,
                    //  we add the row once to the rank buffer
                    if (!rank_set[rankId])
                    {
                        rank_set[rankId] = true;
                        local_int_t t;
#pragma omp atomic capture
                        {
                            t = *p;
                            *p += 1;
                        }
                        send_buffer[rankId * sendbufld + t] = i;
                    }
                }
                else
                {
                    // local neighbor, add it to the local matrix
                    local_int_t zi = cgiz - giz0;
                    local_int_t yi = cgiy - giy0;
                    local_int_t xi = cgix - gix0;
                    local_int_t lcol = zi * ny * nx + yi * nx + xi;
                    mtxIndL[i][nnz_c] = lcol;
                }
                nnz_c++;
            }
        }
    }

    // Now external data structures
    // 1 Create elements to send buffer (Sort the indicies for each neighbor)
    local_int_t totalToBeSent = 0;
    local_int_t* sendcounts = new local_int_t[A.geom->size + 1];
    sendcounts[0] = 0;
    int neighborCount = 0;
#pragma omp parallel for
    for (local_int_t i = 0; i < 27; i++)
    {
        if (sendcounter[i] > 0)
        {
            std::sort(send_buffer + i * sendbufld, send_buffer + i * sendbufld + sendcounter[i]);
        }
    }
    for (local_int_t i = 0; i < 27; i++)
    {
        if (sendcounter[i] > 0)
        {
            totalToBeSent += sendcounter[i];
            sendcounts[neighborCount + 1] = sendcounts[neighborCount] + sendcounter[i];
            neighborCount++;
        }
    }

    // 2 Now find neighbor Ids, neighbor physical Ids (see GenerateGeometry), and elemets to send
    local_int_t sendEntryCount = 0;
    local_int_t* receiveLength = new local_int_t[neighborCount];
    local_int_t* sendLength = new local_int_t[neighborCount];
    // Build the arrays and lists needed by the ExchangeHalo function.
    double* sendBuffer = new double[totalToBeSent];
    int* neighbors = new int[neighborCount];
    int* neighborsPhysical = new int[neighborCount];
    local_int_t* elementsToSend = new local_int_t[totalToBeSent];

    neighborCount = 0;
    for (local_int_t i = 0; i < 27; i++)
    {
        if (sendcounter[i] > 0)
        {
            int neighborId = idToRank_h[i]; // logical Id
            int phys_neiId = logical_rank_to_phys[neighborId];

            neighbors[neighborCount] = neighborId; // store rank ID of current neighbor
            neighborsPhysical[neighborCount] = phys_neiId;
            receiveLength[neighborCount] = sendcounter[i];
            sendLength[neighborCount] = sendcounter[i];

            for (int j = 0; j < sendcounter[i]; j++)
            {
                elementsToSend[sendEntryCount] = send_buffer[i * sendbufld + j];
                sendEntryCount++;
            }
            neighborCount++;
        }
    }

    delete[] send_buffer;
    delete[] sendcounter;

    // Exchange elements to send  wit other neighbors
    auto INDEX_TYPE = MPI_INT;
#ifdef INDEX_64 // In src/Geometry
    INDEX_TYPE = MPI_LONG;
#endif
    MPI_Status status;
    int MPI_MY_TAG = 93;
    MPI_Request* request = new MPI_Request[neighborCount];
    local_int_t* eltsToRecv = new local_int_t[totalToBeSent];
    local_int_t* recv_ptr = eltsToRecv;
    for (int i = 0; i < neighborCount; i++)
    {
        int n_recv = sendLength[i];
        MPI_Irecv(recv_ptr, n_recv, INDEX_TYPE, neighborsPhysical[i], MPI_MY_TAG, MPI_COMM_WORLD, request + i);
        recv_ptr += n_recv;
    }

    local_int_t* elts_ptr = elementsToSend;
    for (int i = 0; i < neighborCount; i++)
    {
        local_int_t n_send = sendLength[i];
        MPI_Send(elts_ptr, n_send, INDEX_TYPE, neighborsPhysical[i], MPI_MY_TAG, MPI_COMM_WORLD);
        elts_ptr += n_send;
    }
    for (int i = 0; i < neighborCount; i++)
    {
        MPI_Wait(request + i, &status);
    }
    delete[] request;

    // Create a map to be used in the optimization step
    //  Any external column index will be given a sequntail Id
    //  after the number of rows (Will be used to access x vector)
    int prev_dim = 0;
    for (int nc = 0; nc < neighborCount; ++nc)
    {
        int neighborId = neighbors[nc];
        int phys_neiId = neighborsPhysical[nc];
        local_int_t str = sendcounts[nc];
        local_int_t end = sendcounts[nc + 1];
        for (int j = str; j < end; j++)
        {
            const local_int_t col = eltsToRecv[j];
            externalToLocalMap[neighborId][col] = localNumberOfRows + j;
        }
    }

    delete[] eltsToRecv;
    delete[] sendcounts;

    if (totalToBeSent > 0)
    {
// Last step sort all external IDs per rank Id, elements of neighbor 0 first, then 1, and so on
#pragma omp parallel for
        for (local_int_t i = 0; i < localNumberOfRows; i++)
        {
            int nnz_ext = 0;
            if (has_external[i] == 1)
            {

                const local_int_t iz = (i / (nx * ny));
                const local_int_t iy = (i - iz * nx * ny) / nx;
                const local_int_t ix = i - (iz * ny + iy) * nx;
                const global_int_t gix = ix + gix0;
                const global_int_t giy = iy + giy0;
                const global_int_t giz = iz + giz0;
                int nnz_c = 0;
                
                for (int k = 0; k < 27; k++)
                {
                    long long int cgix = gix + tid2indCpu[k][0];
                    long long int cgiy = giy + tid2indCpu[k][1];
                    long long int cgiz = giz + tid2indCpu[k][2];

                    local_int_t zi = (cgiz) % nz;
                    local_int_t yi = (cgiy) % ny;
                    local_int_t xi = (cgix) % nx;
                    int ok = cgiz > -1 && cgiz < gnz && cgiy > -1 && cgiy < gny && cgix > -1 && cgix < gnx;
                    int ipz = cgiz / nz;
                    int ipy = cgiy / ny;
                    int ipx = cgix / nx;

                    // The indices sent by the neighbor uses the neighbor's nx, ny, and nz which can
                    // be deffirent than the current neighbor's dims. Thus, based on neighor location
                    // and the diffrent_dim we adjust the indices if needed.
                    // Also, the ipx, ipy, and ipz must be updated accordingly
                    global_int_t new_nx = A.geom->nx;
                    global_int_t new_ny = A.geom->ny;

                    if (A.geom->different_dim == Z)
                    {
                        long long int local = cgiz - giz0;
                        if (local >= 0 && local < nz)
                        {
                            ipz = A.geom->ipz;
                            zi = local;
                        }
                        else if (local < 0)
                        {
                            ipz = A.geom->ipz - 1;
                            zi = A.geom->previous_neighbor_dim - 1;
                        }
                        else if (local >= nz)
                        {
                            ipz = A.geom->ipz + 1;
                            zi = 0;
                        }
                    }
                    else if (A.geom->different_dim == Y)
                    {
                        long long int local = cgiy - giy0;
                        if (local >= 0 && local < ny)
                        {
                            ipy = A.geom->ipy;
                            yi = local;
                        }
                        else if (local < 0)
                        {
                            ipy = A.geom->ipy - 1;
                            yi = A.geom->previous_neighbor_dim - 1;
                            new_ny = A.geom->previous_neighbor_dim;
                        }
                        else if (local >= ny)
                        {
                            ipy = A.geom->ipy + 1;
                            yi = 0;
                            new_ny = A.geom->next_neighbor_dim;
                        }
                    }
                    else if (A.geom->different_dim == X)
                    {
                        long long int local = cgix - gix0;
                        if (local >= 0 && local < nx)
                        {
                            ipx = A.geom->ipx;
                            xi = local;
                        }
                        else if (local < 0)
                        {
                            ipx = A.geom->ipx - 1;
                            xi = A.geom->previous_neighbor_dim - 1;
                            new_nx = A.geom->previous_neighbor_dim;
                        }
                        else if (local >= nx)
                        {
                            ipx = A.geom->ipx + 1;
                            xi = 0;
                            new_nx = A.geom->next_neighbor_dim;
                        }
                    }
                    local_int_t lcol = zi * new_ny * new_nx + yi * new_nx + xi;
                    int row_rank = ipx + ipy * npx + ipz * npy * npx;

                    if (ok)
                    {
                        if (externalToLocalMap.find(row_rank) != externalToLocalMap.end())
                        {
                            mtxIndL[i][nnz_c] = externalToLocalMap[row_rank][lcol];
                            nnz_ext++;
                        }
                        nnz_c++;
                    }
                }
            }
             extTemp[i] = nnz_ext;
        }
    }

    if (P2P_Mode == MPI_CPU_All2allv)
    {
        int* sdispls = new int[A.geom->size];
        int* rdispls = new int[A.geom->size];
        int* scounts = new int[A.geom->size];
        int* rcounts = new int[A.geom->size];
        int tmp_s = 0, tmp_r = 0;

        if (sdispls == NULL || rdispls == NULL || scounts == NULL || rcounts == NULL)
            return;

        for (local_int_t i = 0; i < A.geom->size; i++)
        {
            scounts[i] = 0;
            rcounts[i] = 0;
            sdispls[i] = 0;
            rdispls[i] = 0;
        }

        for (local_int_t i = 0; i < neighborCount; i++)
        {
            local_int_t root = neighborsPhysical[i];
            scounts[root] = sendLength[i];
            rcounts[root] = receiveLength[i];
            sdispls[root] = tmp_s;
            tmp_s += sendLength[i];
            rdispls[root] = tmp_r;
            tmp_r += receiveLength[i];
        }
        A.scounts = scounts;
        A.rcounts = rcounts;
        A.sdispls = sdispls;
        A.rdispls = rdispls;
    }

    delete[] has_external;

    // Store contents in our matrix struct
    A.numberOfExternalValues = totalToBeSent;
    A.localNumberOfColumns = A.localNumberOfRows + A.numberOfExternalValues;
    A.numberOfSendNeighbors = neighborCount;
    A.totalToBeSent = totalToBeSent;
    A.elementsToSend = elementsToSend;
    A.neighbors = neighbors;
    A.neighborsPhysical = neighborsPhysical;
    A.receiveLength = receiveLength;
    A.sendLength = sendLength;
    A.sendBuffer = sendBuffer;
    A.cpuAux.tempIndex = extTemp;

#ifdef HPCG_DETAILED_DEBUG
    HPCG_fout << " For rank " << A.geom->rank << " of " << A.geom->size
              << ", number of neighbors = " << A.numberOfSendNeighbors << endl;
    for (int i = 0; i < A.numberOfSendNeighbors; i++)
    {
        HPCG_fout << "     rank " << A.geom->rank << " neighbor " << neighbors[i]
                  << " send/recv length = " << sendLength[i] << "/" << receiveLength[i] << endl;
        for (local_int_t j = 0; j < sendLength[i]; ++j)
            HPCG_fout << "       rank " << A.geom->rank << " elementsToSend[" << j << "] = " << elementsToSend[j]
                      << endl;
    }
#endif

#endif
    // ifdef HPCG_NO_MPI

    return;
}
#endif // USE_GRACE

void SetupHalo(SparseMatrix& A)
{
    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
        SetupHalo_Gpu(A);
#endif
    }
    else
    {
#ifdef USE_GRACE
        SetupHalo_Cpu(A);
#endif
    }
}