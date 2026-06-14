#!/bin/bash
# Generic tester script
# Only run mpi from rank 0
function test_one_sided() {
    RUN_CMD="${BASE_RUN_CMD} ${@}"
    echo "${RUN_CMD}"
    ${RUN_CMD}
    CODE=$?
    echo "Return code: ${CODE}"
    [[ "${CODE}" -ne 0 ]] && exit 1
}

if [[ "${SLURM_PROCID}" -eq 0 ]]; then
    WINDOW_OPTIONS=(create allocate dynamic)
    WINDOW_SYNCS=(pscw fence lock flush flush_local lock_all)
    PPN=$SLURM_NTASKS_PER_NODE
    NODES=$SLURM_NNODES
    NP=$((PPN*NODES))
    MPI_RUN=$1
    shift
    PARAMS=$@
    echo "-------------------- $SLURM_JOB_ID --------------------"
    echo "Hosts:"
    scontrol show hostname
    BASE_RUN_CMD="${MPI_RUN} -np $NP -ppn $PPN ${@}"

    test_one_sided
    for w in "${WINDOW_OPTIONS}"; do
        test_one_sided -w $w
    done
    for s in "${WINDOW_SYNCS}"; do
        test_one_sided -s $s
    done
    for w in "${WINDOW_OPTIONS}"; do
        for s in "${WINDOW_SYNCS}"; do
            test_one_sided -s $s -w $w
        done
    done
    echo "-------------------- $SLURM_JOB_ID --------------------"
fi
exit 0

