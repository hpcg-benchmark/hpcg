#!/bin/bash
# Generic tester script
# Only run mpi from rank 0
if [[ "${SLURM_PROCID}" -eq 0 ]]; then
    function test_mvp() {
        RUN_CMD="${BASE_RUN_CMD} ${@}"
        echo "${RUN_CMD}"
        ${RUN_CMD}
        CODE=$?
        echo "Return code: ${CODE}"
        [[ "${CODE}" -ne 0 ]] && exit 1
    }
    function run_tests() {
        test_mvp -D cont
        test_mvp -D vect:24:2
        test_mvp -D indx:$DDT_FILE
    }
    DDT_FILE=$PWD/c/util/ddt_sample.txt
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
    run_tests
    echo "-------------------- $SLURM_JOB_ID --------------------"
fi
exit 0
