#!/bin/bash
#SBATCH -o ./result/SBATCH/job.%j.out
#SBATCH -J HPCG_g7_SZW
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=64

export OMP_NUM_THREADS=4
mpiexec -n 32 /public1/home/sc81071/CAS_PP2024/sichengtaotao/hpcg/PP_MPI_OpenMP/bin/xhpcg
