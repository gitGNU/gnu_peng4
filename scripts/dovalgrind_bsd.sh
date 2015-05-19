#! /usr/local/bin/bash

gentestdata()
{
  find /usr/src/linux/ -name "*.c" |while read a
  do
    cat "$a"
  done |buffer
}

#ulimit -c unlimited
set -e

PROXY=valgrind

PARM=1024,8,8
V=-vvv

TESTFILE=testfile

[[ -e $TESTFILE ]] || (gentestdata |dd of=$TESTFILE bs=1024 count=65536 iflag=fullblock)

$PROXY ./peng -n -O $PARM $V -P blablaBLOEBLOE $TESTFILE
#cp ${TESTFILE}.enc ${TESTFILE}_dec_2
#cp ${TESTFILE}.enc ${TESTFILE}_dec_1
#$PROXY ./peng -R -d -O $PARM $V -P blablaBLOEBLOE ${TESTFILE}_dec_1
#$PROXY ./peng -R -d -O $PARM $V -P blablaBLOEBLuE ${TESTFILE}_dec_2
