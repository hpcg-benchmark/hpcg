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

#On a single CG4 with 500GB total memory, the configuration with 
#highest performance (257 GFLOP/s with 54 optimized itertaions) is as follows:

#Local problem size
pnx=128
pny=128
pnz=256

cpu_aff="0-3:4-7:8-11:12-15:16-19:20-23:24-27:28-31:32-35:36-39:40-43:44-47:48-51:52-55:56-59:60-63:72-75:76-79:80-83:84-87:88-91:92-95:96-99:100-103:104-107:108-111:112-115:116-119:120-123:124-127:128-131:132-135:144-147:148-151:152-155:156-159:160-163:164-167:168-171:172-175:176-179:180-183:184-187:188-191:192-195:196-199:200-203:204-207:216-219:220-223:224-227:228-231:232-235:236-239:240-243:244-247:248-251:252-255:256-259:260-263:264-267:268-271:272-275:276-279"
    
mem_aff="0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:1:1:1:1:1:1:1:1:1:1:1:1:1:1:1:1:2:2:2:2:2:2:2:2:2:2:2:2:2:2:2:2:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3"


docker run \
    --privileged \
    -it "$container" \
    mpirun --allow-run-as-root -np 64 --mca pml ^ucx --mca btl ^openib,smcuda -mca coll_hcoll_enable 0 -x coll_hcoll_np=0 --bind-to none \
        /workspace/hpcg-aarch64.sh --nx $pnx --ny $pny --nz $pnz --exm 1 --rt $1 --cpu-affinity $cpu_aff --mem-affinity $mem_aff
