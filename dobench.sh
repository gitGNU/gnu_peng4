#! /bin/bash

gentestdata()
{
  find /usr/src/linux/ -name "*.c" |while read a
  do
    cat "$a"
  done |buffer
}

#ulimit -c unlimited
set -e

#PROXY=valgrind

TESTFILE=testfile

[[ -e $TESTFILE ]] || (gentestdata |dd of=$TESTFILE bs=1024 count=65536 iflag=fullblock)

$PROXY ./slowpeng $TESTFILE ${TESTFILE}_1_enc blablaBLAEH e
./slowpeng ${TESTFILE}_1_enc ${TESTFILE}_1_1_dec blablaBLAEH d
./slowpeng ${TESTFILE}_1_enc ${TESTFILE}_1_2_dec blabluBLORG d
