#!/bin/bash

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

if [[ -z "${NVPL_SPARSE}" ]]; then
    export NVPL_SPARSE=/path/to/nvpllibs #Change this to correct NVPL mathlibs
fi

#Please fix, if needed
export CUDA_BLAS_VERSION=${CUDA_BUILD_VERSION:-12.2}
export LD_LIBRARY_PATH=${MATHLIBS_PATH}/${CUDA_BLAS_VERSION}/lib64/:${LD_LIBRARY_PATH}
export PATH=${CUDA_PATH}/bin:${PATH}
export LD_LIBRARY_PATH=${CUDA_PATH}/lib64:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${NCCL_PATH}/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${NVPL_SPARSE}/lib:${LD_LIBRARY_PATH}

ext="--mca pml ^ucx --mca btl ^openib,smcuda -mca coll_hcoll_enable 0 -x coll_hcoll_np=0 --bind-to none"

#Directory to xhpcg binary
dir="bin/"

#Sample on a Hopper GPU x86
###########################
#Local problem size
nx=512 #Large problem size x
ny=512 #Large problem size y
nz=288 #Large problem size z
mpirun --oversubscribe ${ext} -np 1 ${dir}/hpcg.sh  --exec-name ${dir}/xhpcg \
 --nx $nx --ny $ny --nz $nz --rt 10 --b 0
########################################################################################

#Sample on Grace Hopper x4
###########################
#Local problem size
nx=256 #Large problem size x, assumed for the GPU
ny=1024 #Large problem size y, assumed for the GPU
nz=288 #Large problem size z, assumed for the GPU

#1 GPUOnly
#---------#
np=4  #Total number of ranks
mpirun --oversubscribe ${ext} -np $np ${dir}/hpcg-aarch64.sh  --exec-name ${dir}/xhpcg \
 --nx $nx --ny $ny --nz $nz --rt 10 --b 0 --exm 0 --p2p 0 \
 --mem-affinity 0:1:2:3 --cpu-affinity 0-71:72-143:144-215:216-287

#2 GraceOnly
#-----------#
np=4  #Total number of ranks
mpirun --oversubscribe ${ext} -np $np ${dir}/hpcg-aarch64.sh  --exec-name ${dir}/xhpcg-cpu \
 --nx $nx --ny $ny --nz $nz --rt 10 --b 0 --exm 0 --p2p 0 \
 --mem-affinity 0:1:2:3 --cpu-affinity 0-71:72-143:144-215:216-287

#3 Hetrogeneous (GPU + Grace)
#----------------------------#
np=8  #Total number of ranks (4GPU + 4Grace)
exm=2 #Execution mode GPU+Grace
diff_dim=2 #different dim between GPU and Grace is Y
lpm=1 #Local problem mode (nx/ny/nz are local to GPU, g2c is the Grace different dimension)
g2c=64 #Based on dif_dim=2 and lpm=1 --> Grace rank local problem size is $nx x $g2c x $nz

#3D grid size 4x2x1 (must be equal to np)
npx=4 #number of ranks in the x direction
npy=2 #number of ranks in the y direction
npz=1 #number of ranks in the z direction
mpirun --oversubscribe ${ext} -np $np ${dir}/hpcg-aarch64.sh  --exec-name ${dir}/xhpcg \
 --nx $nx --ny $ny --nz $nz --rt 10 --b 0 --p2p 0 --exm $exm --lpm $lpm --g2c $g2c --ddm $diff_dim --npx $npx --npy $npy --npz $npz \
 --mem-affinity 0:0:1:1:2:2:3:3 --cpu-affinity 0-7:8-71:72-79:80-143:144-151:152-215:216-223:224-287