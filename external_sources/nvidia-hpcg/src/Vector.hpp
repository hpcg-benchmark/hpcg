
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
 @file Vector.hpp

 HPCG data structures for dense vectors
 */

#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cassert>
#include <cstdlib>
#include <cuda_runtime.h>
#include <omp.h>
#include <vector>

#include "Geometry.hpp"

struct Vector_STRUCT
{
    rank_type_t rt;
    local_int_t localLength; //!< length of local portion of the vector
    bool isCudaHost;
    double* values; //!< array of values
    /*!
     This is for storing optimized data structures created in OptimizeProblem and
     used inside optimized ComputeSPMV().
     */
    void* optimizationData;
#ifdef USE_CUDA
    double* values_d = nullptr;
#endif

    bool initialized = false;
};
typedef struct Vector_STRUCT Vector;

/*!
  Initializes input vector.

  @param[in] v
  @param[in] localLength Length of local portion of input vector
 */

inline void InitializeVector(Vector& v, local_int_t localLength, rank_type_t rt, bool isCudaHost = false)
{
    v.localLength = localLength;
    v.isCudaHost = isCudaHost;
    v.rt = rt;

#ifdef USE_CUDA
    if (v.rt == GPU && v.isCudaHost)
        cudaMallocHost(&(v.values), sizeof(double) * localLength);
    else
#endif
        v.values = new double[localLength];

    v.optimizationData = 0;
#ifdef USE_CUDA
    if (v.rt == GPU)
        cudaMalloc((void**) &(v.values_d), sizeof(double) * localLength);
#endif
    v.initialized = true;
    return;
}

/*!
  Fill the input vector with zero values.

  @param[inout] v - On entrance v is initialized, on exit all its values are zero.
 */

inline void ZeroVector(Vector& v)
{

    assert(v.initialized);

    local_int_t localLength = v.localLength;
    double* vv = v.values;
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
    for (local_int_t i = 0; i < localLength; ++i)
        vv[i] = 0.0;
#ifdef USE_CUDA
    if (v.rt == GPU)
    {
        cudaMemset(v.values_d, 0, sizeof(double) * localLength);
    }
#endif
    return;
}
/*!
  Multiply (scale) a specific vector entry by a given value.

  @param[inout] v Vector to be modified
  @param[in] index Local index of entry to scale
  @param[in] value Value to scale by
 */
inline void ScaleVectorValue(Vector& v, local_int_t index, double value)
{
    assert(index >= 0 && index < v.localLength);
    double* vv = v.values;
    vv[index] *= value;
    return;
}
/*!
  Fill the input vector with pseudo-random values.

  @param[in] v
 */
inline void FillRandomVector(Vector& v)
{
    local_int_t localLength = v.localLength;
    double* vv = v.values;
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
    for (local_int_t i = 0; i < localLength; ++i)
        vv[i] = rand() / (double) (RAND_MAX) + 1.0;
    return;
}

/*!
  Copy input vector to output vector.

  @param[in] v Input vector
  @param[in] w Output vector
 */
inline void CopyVector(const Vector& v, Vector& w)
{
    local_int_t len = std::min(v.localLength, w.localLength);
    assert(v.initialized && w.initialized);
    double* vv = v.values;
    double* wv = w.values;
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
    for (local_int_t i = 0; i < len; ++i)
        wv[i] = vv[i];
#ifdef USE_CUDA
    if (v.rt == GPU && w.rt == GPU)
    {
        cudaMemcpy(w.values_d, v.values_d, sizeof(double) * len, cudaMemcpyDeviceToDevice);
    }
#endif
    return;
}

#ifdef USE_CUDA
inline void CopyVectorD2H(const Vector& v)
{
    local_int_t localLength = v.localLength;
    cudaMemcpy(v.values, v.values_d, sizeof(double) * localLength, cudaMemcpyDeviceToHost);
    return;
}

inline void CopyVectorD2D(const Vector& v, Vector& w)
{
    local_int_t localLength = v.localLength;
    cudaMemcpy(w.values_d, v.values_d, sizeof(double) * localLength, cudaMemcpyDeviceToDevice);
    return;
}

inline void CopyVectorH2D(const Vector& v)
{
    local_int_t localLength = v.localLength;
    cudaMemcpy(v.values_d, v.values, sizeof(double) * localLength, cudaMemcpyHostToDevice);
    return;
}
#endif

inline void CopyAndReorderVector(const Vector& v, Vector& w, local_int_t* perm)
{
    local_int_t localLength = v.localLength;
    assert(w.localLength >= localLength);
    double* vv = v.values;
    double* wv = w.values;
    local_int_t i;
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
    for (i = 0; i < localLength; ++i)
    {
        wv[i] = vv[perm[i]];
    }
    return;
}

/*!
  Deallocates the members of the data structure of the known system matrix provided they are not 0.

  @param[in] A the known system matrix
 */
inline void DeleteVector(Vector& v)
{
    if (v.isCudaHost)
        cudaFreeHost(v.values);
    else
    {
        delete[] v.values;
    }
    v.localLength = 0;
#ifdef USE_CUDA
    if (v.values_d)
        cudaFree(v.values_d);
#endif
    return;
}

#endif // VECTOR_HPP
