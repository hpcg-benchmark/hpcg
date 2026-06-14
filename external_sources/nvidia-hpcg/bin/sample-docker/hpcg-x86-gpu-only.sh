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

#To run HPCG-NVIDIA on a single node and 8 GPUs on x86:

#Local problem size
pnx=512
pny=512
pnz=288

docker run \
    --privileged \
    -it "$container" \
    mpirun -np 8 --bind-to none \
        /workspace/hpcg.sh --nx $pnx --ny $pny --nz $pnz --rt $1
