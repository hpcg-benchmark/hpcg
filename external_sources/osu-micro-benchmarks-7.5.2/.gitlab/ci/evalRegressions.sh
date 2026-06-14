#!/bin/bash

PYARGS=""
while [[ $# -gt 0 ]]; do
    PYARGS="${PYARGS} ${1}"
    shift
done

source $HOME/miniconda3/bin/activate
conda activate tuning-suite
THRESHOLD=5
REGRESSION_DIR=$PWD
NEGATIVES=$REGRESSION_DIR/negatives.txt
HTML_FILE="$PWD/report${CI_COMMIT_SHA}.html"

echo "Generating report"
python main.py --report ${PYARGS}
cp $HTML_FILE $CI_PROJECT_DIR
echo "Evaluating..."
if [[ "$(grep 'version failed to run' $HTML_FILE|wc -l)" -ne 0 ]]; then
  echo "Error when evaluating $HTML_FILE:"
  grep -o 'The \(new\|old\) version failed to run' $HTML_FILE | sort | uniq
  exit 1
fi
grep -o '\-[0-9]*.[0-9]*%' $HTML_FILE | grep -o '\-[0-9]*' > $NEGATIVES
#Check number of degredations at or above 10%
NUMBAD=$(awk '{ if($0 < -9) print $0;}' $NEGATIVES | wc -l)

echo "Number of degredations over 10%: "
echo $NUMBAD
if [ "$NUMBAD" -gt "$THRESHOLD" ]; then
  echo "This is more than the allowed threshold of $THRESHOLD";
  exit 1
else
  echo "This is within the allowed limit of $THRESHOLD";
fi

rm $NEGATIVES
cp slurm-* $CI_PROJECT_DIR

echo "All done"
