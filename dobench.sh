#! /bin/bash

ulimit -c unlimited
set -e -x

PROXY=/usr/bin/time

#PARM=32760,19,19
PARM=15000,19,19
V=-vv

TESTSOURCE=testdata/c_64mib
TESTFILE=testfile

cp $TESTSOURCE $TESTFILE

$PROXY ./peng -n -O $PARM $V -P blablaBLOEBLOE $TESTFILE
cp ${TESTFILE}.enc ${TESTFILE}_dec_2
cp ${TESTFILE}.enc ${TESTFILE}_dec_1
./peng -R -d -O $PARM $V -P blablaBLOEBLOE ${TESTFILE}_dec_1
./peng -R -d -O $PARM $V -P blablaBLOEBLuE ${TESTFILE}_dec_2

cp $TESTFILE ${TESTFILE}_aes
$PROXY mcrypt -a aes -m cbc ${TESTFILE}_aes

echo
md5sum ${TESTFILE}*
