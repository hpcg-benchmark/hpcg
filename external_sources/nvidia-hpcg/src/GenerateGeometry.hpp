
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

#ifndef GENERATEGEOMETRY_HPP
#define GENERATEGEOMETRY_HPP
#include "Geometry.hpp"
#include "hpcg.hpp"
void GenerateGeometry(HPCG_Params& params, Geometry* geom);
void GenerateGeometry(int size, int rank, int numThreads, local_int_t nx, local_int_t ny, local_int_t nz, int npx,
    int npy, int npz, dim_3d_t partition_by, Geometry* geom);
#endif // GENERATEGEOMETRY_HPP
