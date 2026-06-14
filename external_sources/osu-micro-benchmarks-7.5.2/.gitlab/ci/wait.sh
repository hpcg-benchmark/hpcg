#!/bin/bash
if [[ $# -ne 1 ]]; then
    echo "ERROR: Must specify a file."
    exit 1
fi
FILE=$1
WAIT_PERIOD=10
echo "~ -_- ~ Waiting for: ${FILE}"
while : ; do
    if [[ -f "${FILE}" ]]; then
        echo "${1} completion file detected, removing"
        rm -f "${FILE}"
        break
    else
        sleep $WAIT_PERIOD
    fi
done
echo "~ 0_0 ~ Done waiting mate"
