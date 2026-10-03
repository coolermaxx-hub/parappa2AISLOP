#!/bin/bash
# nmtest2.sh src/file unit func : compile the whole file with NON_MATCHING defined and diff one function
cd "$(dirname "$0")/../.."
cp $1 /tmp/nmtest_bak
sed -i '1i #define NON_MATCHING' $1
source .venv/bin/activate
tools/dev/fdc.sh $2 $3 > /tmp/nm_out.txt 2>&1
grep -c '|\|<\|>' /tmp/nm_out.txt
cp /tmp/nmtest_bak $1
