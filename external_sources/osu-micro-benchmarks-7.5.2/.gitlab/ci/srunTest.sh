#!/bin/bash
# Generic tester script
# Only run mpi from rank 0
if [[ "${SLURM_PROCID}" -eq 0 ]]; then
    PPN=$SLURM_NTASKS_PER_NODE
    NODES=$SLURM_NNODES
    NP=$((PPN*NODES))
    MPI_RUN=$1
    shift
    RUN_CMD="${MPI_RUN} -np $NP -ppn $PPN ${@}"
    echo "------ $SLURM_JOB_ID - $SLURM_JOB_NAME ------"
    echo "${RUN_CMD}"
    echo "Hosts:"
    scontrol show hostname
    ${RUN_CMD}
    CODE=$?
    echo "Return code: ${CODE}"
    echo "------ $SLURM_JOB_ID - $SLURM_JOB_NAME ------"
    [[ "${CODE}" -ne 0 ]] && exit 1
fi
exit 0
