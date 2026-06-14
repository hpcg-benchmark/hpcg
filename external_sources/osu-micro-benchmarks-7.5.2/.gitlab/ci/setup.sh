#!/bin/bash
export EXPECTED_HOST="ri2"
export MERGE_DIR=/home/gitlab-runner/omb-merges
export BUILD_DIR=$MERGE_DIR/builds
export BUILD_DIR=$BUILD_DIR/$CI_COMMIT_REF_NAME/$CI_COMMIT_SHORT_SHA
export BUILDER="MR_ri2_master"
export BRANCH=$CI_COMMIT_REF_NAME
export LATEST_DIR=$BUILD_DIR/LATEST
export MASTER_DIR=$BUILD_DIR/MASTER
export REG_DIR=$MERGE_DIR/tuning-suite/regression
export MVP_PARAMS="MV2_HOMOGENEOUS_CLUSTER=1"

CLUSTER=$(hostname)

if [[ $CLUSTER == "${EXPECTED_HOST}" ]]; then
    echo "cluster is RI2"
    CLUSTER="RI2"
    CLUSTER_ABBREV="ri2"
    declare -a channels=("ofi")
    ml load mvapich2-2.3.7/gcc-7.5.0
    ml list
else
    echo "OMB CI/CD runner is expecting to run on ${EXPECTED_HOST}"
    exit 1
fi
export CLUSTER=$CLUSTER
export CLUSTER_ABBREV=$CLUSTER_ABBREV
echo "~~ Cluster is $CLUSTER ~~ $CLUSTER_ABBREV"

export REG_WORK_DIR=/home/gitlab-runner/omb-merges/tuning-suite/regression
export INI_TMPL=$GIT_CLONE_PATH/.gitlab/ci/settingsRI2.ini
export INI_NAME=$REG_WORK_DIR/settings.ini

export OMB_PATH=$LATEST_DIR/libexec/osu-micro-benchmarks/mpi
export PT2PT_PATH=$LATEST_DIR/libexec/osu-micro-benchmarks/mpi/pt2pt
export PT2PT_PERSISTENT_PATH=$PT2PT_PATH/persistent
export COLLECTIVE_PATH=$LATEST_DIR/libexec/osu-micro-benchmarks/mpi/collective
export ONE_SIDED_PATH=$LATEST_DIR/libexec/osu-micro-benchmarks/mpi/one-sided
export STARTUP_PATH=$LATEST_DIR/libexec/osu-micro-benchmarks/mpi/startup


export PATH=$PATH:$PWD/.gitlab/ci
export SLUG="${CI_JOB_NAME_SLUG}-${CI_JOB_ID}"
echo "Slug: $SLUG"

function merge_test_out() {
    cat $CI_PROJECT_DIR/OMB-$SLUG*.out>$CI_PROJECT_DIR/$1
    rm $CI_PROJECT_DIR/OMB-$SLUG*.out
}

export -f merge_test_out

