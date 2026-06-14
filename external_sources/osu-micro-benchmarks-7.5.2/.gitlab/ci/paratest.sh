#!/usr/bin/bash
#
# Run tests in parallel
# Put paratest in script
# /opt/bash for wait -n
#
PARACOUNT=3
ENV=""
EXE="srunTest.sh"
FAIL="job.fail"
LOCK="job.lock"
touch $LOCK

# Get arguments
while [[ $# -ne 0 ]]; do
    case "$1" in
        -s|--slug)
            SLUG="${SLUG}-${2}"
            shift 2
            ;;
        -np|--np)
            NP="$2"
            shift 2
            ;;
        -ppn|--ppn_list)
            PPN_LIST="$2"
            shift 2
            ;;
        -d|--dir)
            BENCH_PATH="$2"
            shift 2
            ;;
        -b|--benchmarks)
            BENCH_LIST="$2"
            shift 2
            ;;
        -a|--param)
            PARAM="$2"
            shift 2
            ;;
        -e|--env)
            ENV="$2"
            shift 2
            ;;
        -x|--exe)
            EXE="$2"
            shift 2
            ;;
        --paracount)
            PARACOUNT="$2"
            shift 2
            ;;
        *)
            echo "Bad option supplied"
            exit 3
            ;;
    esac
done

echo "Submitting tests"
echo "Count: $PARACOUNT"
echo "Slug: $SLUG"
echo "NP: $NP"
echo "PPNs: $PPN_LIST"
echo "BENCHMARKS: $BENCH_LIST"
echo "BENCHMARK Path: $BENCH_PATH"
echo "Param: $PARAM"
echo "Env: $ENV"
echo "Bash: $BASH_VERSION"
echo "PWD: $PWD"

function run_benchmark() {
    JOB_NAME="OMB-$SLUG-$b"
    echo "Batching $JOB_NAME"
    srun -J $JOB_NAME -p bdw-v100 -t 1:00:00 \
         -N${NODES} --tasks-per-node=$ppn --exclusive $EXE \
         $MVP_RUN $ENV $BENCH_PATH/$b $PARAM &>${JOB_NAME}.out
    CODE=$?
    if [[ $CODE -ne 0 ]]; then
        flock -w 22 -x $LOCK cat ${JOB_NAME}.out
        touch $FAIL
    fi
}

function cleanup_fn() {
    [[ -f $FAIL ]] && rm $FAIL
    [[ -f $LOCK ]] && rm $LOCK
}

function check_failure() {
    if [[ -f $FAIL ]]; then
        echo "Detected failed benchmark!"
        echo "Waiting for all other jobs to finish"
        wait
        cleanup_fn
        echo "Halting!"
        exit 1
    fi
}

for ppn in $PPN_LIST; do
    if [[ $NP -lt $ppn ]]; then
        echo "Unable to run $SLUG with NP $NP < PPN $ppn"
        continue
    fi
    NODES=$((NP/ppn))
    for b in $BENCH_LIST; do
        # Never run more than PARACOUNT jobs simultaneously
        if [[ $(jobs|wc -l) -eq $PARACOUNT ]]; then
            wait -n
        fi
        check_failure
        ( run_benchmark )&
    done
done
wait

check_failure
cleanup_fn
echo "Test $SLUG completed successfully"
