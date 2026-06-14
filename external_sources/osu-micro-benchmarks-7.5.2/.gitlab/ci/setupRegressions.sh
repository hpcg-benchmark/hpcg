#!/bin/bash

echo "Running on"
hostname

cd $REG_WORK_DIR

FILE=settings.ini

if [[ -f "${PWD}/${INI_NAME}" ]]; then
  rm $INI_NAME
fi

if [[ $CI_MERGE_REQUEST_TARGET_BRANCH_NAME == "master" ]] || [[ $PIPELINE_BRANCH_TYPE == "master" ]]; then
  BUILDER_SUFFIX="master"
fi

cp $INI_TMPL $INI_NAME

#Edit Necessary lines in settings.ini file
sed -i 's$Arch: RI2$Arch: Merge-to-'$CI_COMMIT_SHA'$' $INI_NAME

sed -i 's$Path: /old$Path: '$LATEST_DIR'$' $INI_NAME

sed -i 's$Path_Old: /old$Path_Old: '$MASTER_DIR'$' $INI_NAME

sed -i 's$Commit: /old$Commit: '$CI_COMMIT_SHA'$' $INI_NAME

sed -i 's$Commit_Old: /old$Commit_Old: LATEST_'$BUILDER_SUFFIX'$' $INI_NAME

sed -i 's$outfile: report.html$outfile: report'$CI_COMMIT_SHA'.html$' $INI_NAME

sed -i 's$title: old$title: '$CLUSTER' - '$BRANCH' '$CI_COMMIT_SHA' vs Old: LATEST_'$BUILDER_SUFFIX'$' $INI_NAME

#Clean up old slurm files, verification files and reports

if test -n "$(find . -maxdepth 1 -name 'slurm-*' -print -quit)"
then
    echo "Removing old slurm files"
    rm slurm-*
fi
if test -n "$(find . -maxdepth 1 -name '1-nodes*' -print -quit)"
then
    echo "Removing old 1-node verification files"
    rm 1-nodes*
fi
if test -n "$(find . -maxdepth 1 -name '2-nodes*' -print -quit)"
then
    echo "Removing old 2-node verification files"
    rm 2-nodes*
fi
if test -n "$(find . -maxdepth 1 -name 'pt2pt-*' -print -quit)"
then
    echo "Removing old slurm files"
    rm pt2pt-*
fi
if test -n "$(find . -maxdepth 1 -name '*.html' -print -quit)"
then
    echo "Removing old regression reports"
    rm *.html
fi
