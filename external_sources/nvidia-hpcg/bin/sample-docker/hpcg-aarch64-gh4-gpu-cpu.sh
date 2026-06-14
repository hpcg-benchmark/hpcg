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

container=${CONT}

echo "Container name: "$container
set -x
    docker pull "$container"
set +x

#On a single CG4 with 90GB GPU memory and 500GB total memory, the configuration with 
#highest performance (2,659 GFLOP/s with 52 optimized itertaions) is as follows:

#GPU+CPU 
#--exm: (execution mode) specifies the execution mode. 0 is GPU-only, 1 is Grace-only, and 2 is GPU+Grace. default is 0
#--ddm: specifies the direction that GPU and CPU will not have the same dimension. 0 is auto, 1 is x, 2 is y, and 3 is z. default is 0
#--lpm: controls the meaning of the value provided for --g2c parameter. Applicable when --exm is 2
#  0 means nx/ny/nz are GPU dims and g2c is the ratio of GPU dim to Grace dim. e.g., --g2c 8 means the different CPU dim is 1/8 the GPU dim
#  1 means nx/ny/nz are GPU dims and g2c is the abs value for the different dim for Grace. e.g., --g2c 64 means the different CPU dim is 64 
#  2 assumes the sum of different dims of GPU and Grace is part of nx, ny, or nz (depend on --pby) and --g2c is the ratio. e.g., --pby 1, --nx 1024, and --g2c 8 then GPU nx is 896 and Grace nx is 128
#  3 assumes the sum of different dims of GPU and Grace is part of nx, ny, or nz (depend on --pby) and --g2c is absolute. e.g., --pby 1, --nx 1024, and --g2c 96 then GPU nx is 928 and Grace nx is 96 
# --g2c specifies the differnt dimensions of the GPU and Grace ranks. Depends on --lpm value"
# --npx specifies the process grid X dimensions of the problem
# --npy specifies the process grid Y dimensions of the problem
# --npz specifies the process grid Z dimensions of the problem

np=8    #number of ranks
exm=2   #Execution mode is GPU+Grace
nx=256  
ny=1024
nz=288
pby=2 #user-defined dimension that GPU and Grace will have different dimensions. If not set or 0, partition axis is chosen automatically
lpm=1   # NX/NY/NZ are for the GPU rank. NY of the CPU rank will have the absolute value of g2c
g2c=64 #CPU problem size is 256x64x288 
npx=4 #[optional] number of ranks in the x direction
npy=2 #[optional] number of ranks in the y direction
npz=1 #[optional] number of ranks in the z direction

cpu_aff="0-7:8-71:72-79:80-143:144-151:152-215:216-223:224-287"
mem_aff="0:0:1:1:2:2:3:3"

docker run \
    --privileged \
    -it "$container" \
    mpirun --allow-run-as-root -np $np --mca pml ^ucx --mca btl ^openib,smcuda -mca coll_hcoll_enable 0 -x coll_hcoll_np=0 --bind-to none \
        /workspace/hpcg-aarch64.sh --nx $nx --ny $ny --nz $nz --exm 0 --rt $1 \
        --exm $exm --lpm $lpm --g2c $g2c --pby $pby --npx $npx --npy $npy --npz $npz \
         --cpu-affinity $cpu_aff --mem-affinity $mem_aff
