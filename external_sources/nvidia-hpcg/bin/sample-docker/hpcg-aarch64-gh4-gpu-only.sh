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

#On a single CG4 with 90GB GPU memory, the configuration with 
#highest performance (2,659 GFLOP/s with 52 optimized itertaions) is as follows:

#Local problem size
pnx=512
pny=512
pnz=288

cpu_aff="0-71:72-143:144-215:216-287"
    
mem_aff="0:1:2:3"


docker run \
    --privileged \
    -it "$container" \
    mpirun --allow-run-as-root -np 4 --mca pml ^ucx --mca btl ^openib,smcuda -mca coll_hcoll_enable 0 -x coll_hcoll_np=0 --bind-to none \
        /workspace/hpcg-aarch64.sh --nx $pnx --ny $pny --nz $pnz --exm 0 --rt $1 --cpu-affinity $cpu_aff --mem-affinity $mem_aff
