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
 @file CheckAspectRatio.cpp

 HPCG routine
 */

#include <algorithm>
#include <iostream>

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

#include "hpcg.hpp"

#include "CheckAspectRatio.hpp"

extern int use_output_file;

int CheckAspectRatio(double smallest_ratio, int x, int y, int z, const char* what, bool DoIo)
{
    double current_ratio = std::min(std::min(x, y), z) / double(std::max(std::max(x, y), z));

    if (current_ratio < smallest_ratio)
    { // ratio of the smallest to the largest
        if (DoIo)
        {
            if (use_output_file)
            {
                HPCG_fout << "The " << what << " sizes (" << x << "," << y << "," << z
                          << ") are invalid because the ratio min(x,y,z)/max(x,y,z)=" << current_ratio
                          << " is too small (at least " << smallest_ratio << " is required)." << std::endl;
                HPCG_fout << "The shape should resemble a 3D cube. Please adjust and try again." << std::endl;
                HPCG_fout.flush();
            }
            else
            {
                std::cout << "The " << what << " sizes (" << x << "," << y << "," << z
                          << ") are invalid because the ratio min(x,y,z)/max(x,y,z)=" << current_ratio
                          << " is too small (at least " << smallest_ratio << " is required)." << std::endl;
                std::cout << "The shape should resemble a 3D cube. Please adjust and try again." << std::endl
                          << std::flush;
            }
        }

#ifndef HPCG_NO_MPI
        MPI_Abort(MPI_COMM_WORLD, 127);
#endif

        return 127;
    }

    return 0;
}
