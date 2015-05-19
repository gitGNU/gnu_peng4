#! /usr/local/bin/bash

ulimit -c unlimited
set -e

#PROXY=/usr/bin/time
#PROXY=`which valgrind`
PROXY=kdbg

#PARM=32760,19,19
#PARM=8192,6,10
PARM=8192,8,8
V=-vv

TESTSOURCE=testdata/c_64mib
TESTFILE=testfile

cp $TESTSOURCE $TESTFILE

#$PROXY ./peng -n -m -O $PARM $V -P blablaBLOEBLOE $TESTFILE
#cp ${TESTFILE}.enc ${TESTFILE}_dec_3
#cp ${TESTFILE}.enc ${TESTFILE}_dec_2
#cp ${TESTFILE}.enc ${TESTFILE}_dec_1
$PROXY -a "-R -d -O $PARM $V -P blablaBLOEBLOE ${TESTFILE}_dec_1" ./peng
#$PROXY ./peng -R -d -O $PARM $V -P blablaBLOEBLuE ${TESTFILE}_dec_2
#$PROXY ./peng -R -d -O $PARM $V -P SomethingTOTALLYdifferent\! ${TESTFILE}_dec_3

echo
md5sum ${TESTFILE}*
