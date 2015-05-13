#! /bin/bash

ulimit -c unlimited
set -e

PROXY=

PARM=8192,3,3
V=

TESTSOURCE=testdata/c_64mib

#i=$((16777216-1000))
#i9=$(($i-8200))
i=16770566
i9=$((16777216-1000-8200))

TESTFILE=testfile

cp $TESTSOURCE $TESTFILE
rm -f log

while [[ $i -ge $i9 ]]
do

truncate -s $i $TESTFILE

$PROXY ./peng -n -m -O $PARM $V -P blablaBLOEBLOE $TESTFILE
cp ${TESTFILE}.enc ${TESTFILE}_dec_1
$PROXY ./peng -R -m -d -O $PARM $V -P blablaBLOEBLOE ${TESTFILE}_dec_1

(
echo SIZE=$i
md5sum $TESTFILE ${TESTFILE}_dec_1
echo
) |tee -a log

rm -f ${TESTFILE}.enc ${TESTFILE}_dec_1

i=$(($i-1))

done

