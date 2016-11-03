#! /bin/bash

ulimit -c unlimited
set -e

#PROXY=/usr/bin/time
#PROXY=`which valgrind`

for PARM in 32760,19,19 8192,6,10 8192,8,8 7276,19,19
do

ID="`echo $PARM |sed s/,/-/g`"

V=-vv

TESTSOURCE=testdata/h_128mib
TESTFILE=testfile

cp $TESTSOURCE $TESTFILE

ln $TESTFILE ${TESTFILE}_${ID} || true

$PROXY ./peng -m -n -O $PARM $V -P blablaBLOEBLOE ${TESTFILE}_${ID}
#cp ${TESTFILE}.enc ${TESTFILE}_dec_3
cp ${TESTFILE}_${ID}.enc ${TESTFILE}_${ID}_dec_2
cp ${TESTFILE}_${ID}.enc ${TESTFILE}_${ID}_dec_1
$PROXY ./peng -m -R -d -O $PARM $V -P blablaBLOEBLOE ${TESTFILE}_${ID}_dec_1
$PROXY ./peng -m -R -d -O $PARM $V -P blablaBLOEBLuE ${TESTFILE}_${ID}_dec_2 || true
#$PROXY ./peng -m -R -d -O $PARM $V -P SomethingTOTALLYdifferent\! ${TESTFILE}_dec_3

echo
#md5sum ${TESTFILE}*

done 2>&1 |tee dotestrun_log

