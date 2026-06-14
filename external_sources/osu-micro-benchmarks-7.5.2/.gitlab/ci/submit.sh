#!/bin/bash
if [[ $# -ne 1 ]]; then
    echo "Incorrect parameters..."
    exit 1
fi

JOB_NAME=$1
CONDA_BASE_PATH=/home/gitlab-runner/miniconda3
CONDA_TUNE_ENV=tuning-suite
TUNING_REPO=/home/gitlab-runner/omb-merges/tuning-suite

# Set up the conda environment
source ${CONDA_BASE_PATH}/etc/profile.d/conda.sh
conda activate ${CONDA_TUNE_ENV}

# -u flushes stdout after every print statement
# without it the job output only appears when the job completes
python -u ${TUNING_REPO}/regression/main.py --run ${JOB_NAME}

# Touch verify script once it's all done
touch ${JOB_NAME}.txt

