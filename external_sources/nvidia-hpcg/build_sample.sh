#! /usr/bin/env bash

# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

export CXX_PATH=/usr
export PATH=${CXX_PATH}/bin:${PATH}

if [[ -z "${MPI_PATH}" ]]; then
    export MPI_PATH=/path/to/mpi #Change this to correct MPI path
fi

if [[ -z "${CUDA_PATH}" ]]; then
    export MATHLIBS_PATH=/path/to/mathlibs #Change this to correct CUDA mathlibs
fi

if [[ -z "${NCCL_PATH}" ]]; then
    export NCCL_PATH=/path/to/nccl #Change to correct NCCL path
fi

if [[ -z "${CUDA_PATH}" ]]; then
    export CUDA_PATH=/path/to/cuda #Change this to correct CUDA path
fi

if [[ -z "${NVPL_SPARSE_PATH}" ]]; then
    export NVPL_SPARSE_PATH=/path/to/nvpllibs #Change this to correct NVPL mathlibs
fi

export PATH=${CUDA_PATH}/bin:${PATH}
export LD_LIBRARY_PATH=${CUDA_PATH}/lib64:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${NVPL_SPARSE_PATH}/lib:${LD_LIBRARY_PATH}

#xhpcg binary will be located in build/bin
mkdir -p build
cd build

######## USE Nvidia GPU? ############
# 1:         Yes
# O:         No 
export USE_CUDA=1
if [[ $5 == "0" ]]; then
    export USE_CUDA=0
fi
################################################

######## USE Grace CPU? ############
# 1:         Yes
# O:         No 
export USE_GRACE=1
if [[ $6 == "0" ]]; then
    export USE_GRACE=0
fi
################################################

######## USE NCCL? ############
# 1:         Yes
# O:         No 
export USE_NCCL=1
if [[ $7 == "0" ]]; then
    export USE_NCCL=0
fi
################################################

if [[ $USE_GRACE == 1 ]]; then
    ../configure CUDA_AARCH64
else
    ../configure CUDA_X86
fi

export build_B100=0
if [[ $8 == "1" ]]; then
    export build_B100=1
fi

make -j 16 \
    USE_CUDA=${USE_CUDA} \
    USE_GRACE=${USE_GRACE} \
    USE_NCCL=${USE_NCCL} \
    MPdir=${MPI_PATH} \
    MPlib=${MPI_PATH}/lib \
    Mathdir=${MATHLIBS_PATH} \
    NCCLdir=${NCCL_PATH} \
    CUDA_HOME=${CUDA_PATH} \
    NVPL_PATH=${NVPL_SPARSE_PATH} \
    HPCG_ENG_VERSION=${is_ENG_VERSION} \
    HPCG_COMMIT_HASH=$2 \
    HPCG_VER_MAJOR=$3 \
    HPCG_VER_MINOR=$4 \
    BUILD_B100=${build_B100}

#Move build/bin/xhpcg to bin/xhpcg
make install