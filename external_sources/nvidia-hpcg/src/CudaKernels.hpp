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

#pragma once
#ifdef USE_CUDA
#include "SparseMatrix.hpp"

///////// L2 Memory Compression Allocation Support Routines //
cudaError_t setProp(CUmemAllocationProp* prop);
cudaError_t cudaMallocCompressible(void** adr, size_t size);
cudaError_t cudaFreeCompressible(void* ptr, size_t size);

///////// Allocate CUDA Memory for data structures //
local_int_t EstimateLUmem(local_int_t n, local_int_t padded_n, local_int_t level);
void AllocateMemCuda(SparseMatrix& A_in);
void AllocateMemOptCuda(SparseMatrix& A_in);

///////// Deallocate CUDA Memory for data structures //
void DeleteMatrixGpu(SparseMatrix& A);

///////// Genrerate Problem //
void GenerateProblemCuda(SparseMatrix& A, Vector* b, Vector* x, Vector* xexact);

// Halo Exchange
void SetupHaloCuda(SparseMatrix& A, local_int_t sendbufld, local_int_t* sendlen, local_int_t* sendbuff,
    local_int_t* tot_to_send, int* nneighs, int* neighs_h, local_int_t* sendlen_h, local_int_t** elem_to_send_d);
void ExtToLocMapCuda(
    local_int_t localNumberOfRows, local_int_t str, local_int_t end, local_int_t* extToLocMap, local_int_t* eltsToRecv);
void ExtTolocCuda(local_int_t localNumberOfRows, int neighborId, local_int_t ext_nnz, local_int_t* csr_ext_columns,
    double* csr_ext_values, local_int_t* ext2csr_offsets, local_int_t* extToLocMap, local_int_t* csrColumns);
void PackSendBufferCuda(const SparseMatrix& A, Vector& x, bool cpu_data, cudaStream_t stream1);
void ExchangeHaloCuda(const SparseMatrix& A, Vector& x, cudaStream_t stream1, int use_ibarrier = 0);

// Optimize Problem
void SetVectorAscCuda(local_int_t* arr, local_int_t n);
void ColorMatrixCuda(double* A_vals, local_int_t* A_col, local_int_t* nnzPerRow, local_int_t rows, local_int_t* color,
    int* num_colors, int* count_colors, int max_colors, local_int_t* ref2opt, local_int_t* opt2ref, int rank, int nx,
    int* rowhash);
void PermElemToSendCuda(local_int_t totalToBeSent, local_int_t* elementsToSend, local_int_t* perm);
void EllPermColumnsValuesCuda(local_int_t localNumberOfRows, local_int_t* nnzPerRow, local_int_t* csrColumns,
    double* csrValues, local_int_t* permOffsets, local_int_t* permColumns, double* permValues, local_int_t* opt2ref,
    local_int_t* ref2opt, local_int_t* diagonalIdx, local_int_t* permLOffsets, local_int_t* permUOffsets, bool diag);
void TransposeCuda(local_int_t n, local_int_t slice_size, local_int_t* sellCollIndex, double* sellValues);
void EllMaxRowLenPerBlockCuda(local_int_t nrow, int sliceSize, local_int_t* sellLPermOffsets,
    local_int_t* sellUPermOffsets, local_int_t* sellLSliceMrl, local_int_t* sellUSliceMrl);
void PrefixsumCuda(local_int_t localNumberOfRows, local_int_t* arr);
void MultiplyBySliceSizeCUDA(local_int_t nrow, int slice_size, local_int_t* arr);
void CreateAMatrixSliceOffsetsCuda(local_int_t nrow, local_int_t slice_size, local_int_t* arr);
void CreateSellLUColumnsValuesCuda(const local_int_t n, int sliceSize, local_int_t* columns, double* values,
    local_int_t* sellLSliceOffset, local_int_t* sellLColumns, double* sellLValues, local_int_t* sellUSliceOffset,
    local_int_t* sellUColumns, double* sellUValues, int level);
void PermVectorCuda(local_int_t* perm, Vector& x, local_int_t length);
void F2cPermCuda(local_int_t nrow_c, local_int_t* f2c, local_int_t* f2cPerm, local_int_t* permF, local_int_t* ipermC);

// Test CG
void ReplaceMatrixDiagonalCuda(SparseMatrix& A, Vector& diagonal);
void CopyMatrixDiagonalCuda(SparseMatrix& A, Vector& diagonal);

// CG Support Kernels
// 1. MG
void ComputeRestrictionCuda(const SparseMatrix& A, const Vector& r);
void ComputeProlongationCuda(const SparseMatrix& A, Vector& x);

// 2. WAXPBY
void ComputeWAXPBYCuda(
    const local_int_t n, const double alpha, const Vector& x, const double beta, const Vector& y, Vector& w);

// 3.SYMGS
void SpmvDiagCuda(local_int_t n, double* x, double* d);
void AxpbyCuda(local_int_t n, double* x, double* y, double* z);
void SpFmaCuda(local_int_t n, double* x, double* y, double* z);

// 4.External Matrix SpMV + Scatter
void ExtSpMVCuda(SparseMatrix& A, double alpha, double* x, double* y);

// Transfer Problem to CPU
size_t CopyDataToHostCuda(SparseMatrix& A, Vector* b, Vector* x, Vector* xexact);
#endif