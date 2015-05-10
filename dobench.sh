#! /bin/bash

ulimit -c unlimited
set -e

PROXY=/usr/bin/time

#PARM=32760,19,19
PARM=8192,5,8
V=-vvv

TESTSOURCE=testdata/x128mib
TESTFILE=testfile

[[ -e $TESTFILE ]] || cp $TESTSOURCE $TESTFILE

$PROXY ./peng -n -O $PARM $V -P blablaBLOEBLOE $TESTFILE
cp ${TESTFILE}.enc ${TESTFILE}_dec_3
cp ${TESTFILE}.enc ${TESTFILE}_dec_2
cp ${TESTFILE}.enc ${TESTFILE}_dec_1
$PROXY ./peng -R -d -O $PARM $V -P blablaBLOEBLOE ${TESTFILE}_dec_1
$PROXY ./peng -R -d -O $PARM $V -P blablaBLOEBLuE ${TESTFILE}_dec_2
$PROXY ./peng -R -d -O $PARM $V -P SomethingTOTALLYdifferent\! ${TESTFILE}_dec_3

echo
md5sum ${TESTFILE}*
