#!/usr/bin/env bash

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

SCRIPT_DIR=$( cd -- "$( dirname -- "$( readlink -f "${BASH_SOURCE[0]}" )" )" &> /dev/null && pwd )
XHPCG="$SCRIPT_DIR/xhpcg"

if [[ -r "$SCRIPT_DIR/../hpc-benchmarks-gpu-env.sh" ]]; then
  source "$SCRIPT_DIR/../hpc-benchmarks-gpu-env.sh"
fi

# FIXME - workaround for Singularity
export LD_LIBRARY_PATH="/usr/local/cuda/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

export UCX_WARN_UNUSED_ENV_VARS=n

export OMP_PROC_BIND=TRUE
export OMP_PLACES=sockets

# FIXME - workaround for HPC-X 2.18
export UCC_LOG_LEVEL=error

usage() {
  echo ""
  echo "$(basename "$0") [OPTION]"
  echo "    --dat <string>                path to HPCG.dat"
  echo "    --cuda-compat                 manually enable CUDA forward compatibility"
  echo "    --gpu-affinity <string>       colon separated list of gpu indices"
  echo "    --cpu-affinity <string>       colon separated list of cpu index ranges"
  echo "    --mem-affinity <string>       colon separated list of memory indices"
  echo "    --ucx-affinity <string>       colon separated list of UCX devices"
  echo "    --ucx-tls      <string>       UCX transport to use"
  echo "    --exec-name    <string>       HPCG executable file"
  echo "    --npx          <int>          specifies the process grid X dimension of the problem"
  echo "    --npy          <int>          specifies the process grid Y dimension of the problem"
  echo "    --npz          <int>          specifies the process grid Z dimension of the problem"
  echo "    --p2p          <int>          specifies the p2p comm mode: 0 MPI_CPU, 1 MPI_CPU_All2allv, 2 MPI_CUDA_AWARE, 3 MPI_CUDA_AWARE_All2allv, 4 NCCL. Default MPI_CPU"
  echo "    --b            <int>          activates benchmarking mode to bypass CPU reference execution when set to one (--b 1)"
  echo "    --l2cmp        <int>          activates compression in GPU L2 cache when set to one (--l2cmp 1)"
  echo "    --of           <int>          activates generating the log into textfiles, instead of stdout (--of 1)"
  echo "    --gss          <int>          GPU slice size for sliced-ELLPACK format"
  echo "    --wt           <int>          specifies the number of seconds for the warmup phase before the timed benchmark"
  echo ""
}

info() {
  local msg=$*
  echo -e "INFO: ${msg}"
}

warning() {
  local msg=$*
  echo -e "WARNING: ${msg}"
}

error() {
  local msg=$*
  echo -e "ERROR: ${msg}"
  exit 1
}

# split the affinity string, e.g., '0:2:4:6' into an array,
# e.g., map[0]=0, map[1]=2, ...  The array index is the MPI rank.
read_gpu_affinity_map() {
    local affinity_string=$1
    readarray -t GPU_AFFINITY_MAP <<<"$(tr ':' '\n'<<<"$affinity_string")"
}

# split the affinity string, e.g., '0:2:4:6' into an array,
# e.g., map[0]=0, map[1]=2, ...  The array index is the MPI rank.
read_net_affinity_map() {
    local affinity_string=$1
    readarray -t NET_AFFINITY_MAP <<<"$(tr ':' '\n'<<<"$affinity_string")"
}

# split the affinity string, e.g., '0:2:4:6' into an array,
# e.g., map[0]=0, map[1]=2, ...  The array index is the MPI rank.
read_mem_affinity_map() {
    local affinity_string=$1
    readarray -t MEM_AFFINITY_MAP <<<"$(tr ':' '\n'<<<"$affinity_string")"
}

# split the affinity string, e.g., '0:2:4:6' into an array,
# e.g., map[0]=0, map[1]=2, ...  The array index is the MPI rank.
read_cpu_affinity_map() {
    local affinity_string=$1
    readarray -t CPU_AFFINITY_MAP <<<"$(tr ':' '\n'<<<"$affinity_string")"
}

# set PMIx client configuration to match the server
# enroot already handles this, so only do this under singularity
# https://github.com/NVIDIA/enroot/blob/master/conf/hooks/extra/50-slurm-pmi.sh
set_pmix() {
    if [ -d /.singularity.d ]; then
        if [ -n "${PMIX_PTL_MODULE-}" ] && [ -z "${PMIX_MCA_ptl-}" ]; then
            export PMIX_MCA_ptl=${PMIX_PTL_MODULE}
        fi
        if [ -n "${PMIX_SECURITY_MODE-}" ] && [ -z "${PMIX_MCA_psec-}" ]; then
            export PMIX_MCA_psec=${PMIX_SECURITY_MODE}
        fi
        if [ -n "${PMIX_GDS_MODULE-}" ] && [ -z "${PMIX_MCA_gds-}" ]; then
            export PMIX_MCA_gds=${PMIX_GDS_MODULE}
        fi
    fi
}

### main script starts here

# Read command line arguments
while [ "$1" != "" ]; do
  case $1 in
   --cuda-compat )
      export LD_LIBRARY_PATH="/usr/local/cuda/compat:$LD_LIBRARY_PATH"
      ;;
    --dat )
      if [ -n "$2" ]; then
        DAT="$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
    --exec-name )
      if [ -n "$2" ]; then
        XHPCG=$2
      else
        usage
        exit 1
      fi
      shift
      ;;
   --ucx-tls )
      if [ -n "$2" ]; then
        export UCX_TLS="$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
    --ucx-affinity )
      if [ -n "$2" ]; then
        NET_AFFINITY="$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
    --gpu-affinity )
      if [ -n "$2" ]; then
        GPU_AFFINITY="$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
    --mem-affinity )
      if [ -n "$2" ]; then
        MEM_AFFINITY="$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
    --cpu-affinity )
      if [ -n "$2" ]; then
        CPU_AFFINITY="$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
     --nx )
       if [ -n "$2" ]; then
         NX="$2"
        else
          usage
          exit 1
        fi
        shift
        ;;
     --ny )
       if [ -n "$2" ]; then
         NY="$2"
        else
          usage
          exit 1
        fi
        shift
        ;;
     --nz )
       if [ -n "$2" ]; then
         NZ="$2"
        else
          usage
          exit 1
        fi
        shift
        ;;
     --rt )
       if [ -n "$2" ]; then
         RT="$2"
        else
          usage
          exit 1
        fi
        shift
        ;;
     --b )
       if [ -n "$2" ]; then
         B="--b=$2"
        else
          usage
          exit 1
        fi
        shift
        ;;
     --l2cmp )
       if [ -n "$2" ]; then
         L2CMP="--l2cmp=$2"
        else
          usage
          exit 1
        fi
        shift
        ;;
     --of )
      if [ -n "$2" ]; then
        OF="--of=$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
    --npx )
      if [ -n "$2" ]; then
        NPX="--npx=$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
    --npy )
      if [ -n "$2" ]; then
        NPY="--npy=$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
    --npz )
      if [ -n "$2" ]; then
        NPZ="--npz=$2"
      else
        usage
        exit 1
      fi
      shift
      ;;  
     --p2p )
       if [ -n "$2" ]; then
         P2P="--p2p=$2"
       else
         usage
         exit 1
       fi
       shift
       ;;
     --gss )
      if [ -n "$2" ]; then
        GSS="$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
     --wt )
      if [ -n "$2" ]; then
        WT="--wt=$2"
      else
        usage
        exit 1
      fi
      shift
      ;;
  * )
    usage
    exit 1
  esac
  shift
done

# Setup PMIx, if using
if [[ -z "${SLURM_MPI_TYPE-}" || "${SLURM_MPI_TYPE}" == pmix* ]]; then
  set_pmix
fi

read_rank() {
  # Global rank
  if [ -n "${OMPI_COMM_WORLD_RANK}" ]; then
    RANK=${OMPI_COMM_WORLD_RANK}
  elif [ -n "${PMIX_RANK}" ]; then
    RANK=${PMIX_RANK}
  elif [ -n "${PMI_RANK}" ]; then
    RANK=${PMI_RANK}
  elif [ -n "${SLURM_PROCID}" ]; then
    RANK=${SLURM_PROCID}
  else
    warning "could not determine rank"
  fi

  # Node local rank
  if [ -n "${OMPI_COMM_WORLD_LOCAL_RANK}" ]; then
    LOCAL_RANK=${OMPI_COMM_WORLD_LOCAL_RANK}
  elif [ -n "${SLURM_LOCALID}" ]; then
    LOCAL_RANK=${SLURM_LOCALID}
  else
    error "could not determine local rank"
  fi
}

if [[ -n "${NET_AFFINITY}" ]]; then
  read_rank
  read_net_affinity_map $NET_AFFINITY
  NET=${NET_AFFINITY_MAP[$LOCAL_RANK]}
  if [ -n "${NET}" ]; then
    export UCX_NET_DEVICES="$NET:1"
  fi
fi

if [[ -n "${GPU_AFFINITY}" ]]; then
  read_rank
  read_gpu_affinity_map $GPU_AFFINITY
  GPU=${GPU_AFFINITY_MAP[$LOCAL_RANK]}
  export CUDA_VISIBLE_DEVICES=${GPU}
fi

if [[ -n "${MEM_AFFINITY}" ]]; then
  read_rank
  read_mem_affinity_map $MEM_AFFINITY
  MEM=${MEM_AFFINITY_MAP[$LOCAL_RANK]}
  MEMBIND="--membind=${MEM}"
fi

if [[ -n "${CPU_AFFINITY}" ]]; then
  read_rank
  read_cpu_affinity_map $CPU_AFFINITY
  CPU=${CPU_AFFINITY_MAP[$LOCAL_RANK]}
  CPUBIND="--physcpubind=${CPU}"
fi

if [ -n "${MEMBIND}" ] || [ -n "${CPUBIND}" ]; then
  NUMCMD="numactl "
fi

# if [ $LOCAL_RANK -eq 0 ]; then
#   sudo nvidia-smi -lgc ${GPU_CLOCK}
# fi
# export CUDA_VISIBLE_DEVICES=${GPU}

HPCG_CONTROL="${B} ${L2CMP} ${P2P} ${OF} ${NPX} ${NPY} ${NPZ} ${WT}"

if [[ -z "${NX}" || -z "${NY}" || -z "${NZ}" || -z "${RT}" ]]; then
    if [ -z "${DAT}" ]; then
        error "DAT file not provided"
    fi
    ${NUMCMD} ${CPUBIND} ${MEMBIND} ${XHPCG} ${DAT} --gss=${GSS} ${HPCG_CONTROL}
else
    ${NUMCMD} ${CPUBIND} ${MEMBIND} ${XHPCG} --nx=${NX} --ny=${NY} --nz=${NZ} --rt=${RT} --gss=${GSS} ${HPCG_CONTROL}
fi