
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
 @file CGData.hpp

 HPCG data structure
 */

#ifndef CGDATA_HPP
#define CGDATA_HPP

#include "SparseMatrix.hpp"
#include "Vector.hpp"

struct CGData_STRUCT
{
    Vector r;  //!< pointer to residual vector
    Vector z;  //!< pointer to preconditioned residual vector
    Vector p;  //!< pointer to direction vector
    Vector Ap; //!< pointer to Krylov vector
};
typedef struct CGData_STRUCT CGData;

/*!
 Constructor for the data structure of CG vectors.

 @param[in]  A    the data structure that describes the problem matrix and its structure
 @param[out] data the data structure for CG vectors that will be allocated to get it ready for use in CG iterations
 */
inline void InitializeSparseCGData(SparseMatrix& A, CGData& data)
{
    local_int_t nrow = A.localNumberOfRows;
    local_int_t ncol = A.localNumberOfColumns;
    InitializeVector(data.r, nrow, A.rankType);
    InitializeVector(data.z, ncol, A.rankType, true /*Only when rank type is GPU*/);
    InitializeVector(data.p, ncol, A.rankType, true);
    InitializeVector(data.Ap, nrow, A.rankType);
    return;
}

/*!
 Destructor for the CG vectors data.

 @param[inout] data the CG vectors data structure whose storage is deallocated
 */
inline void DeleteCGData(CGData& data)
{
    DeleteVector(data.r);
    DeleteVector(data.z);
    DeleteVector(data.p);
    DeleteVector(data.Ap);
    return;
}

#endif // CGDATA_HPP
