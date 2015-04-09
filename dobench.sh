#! /bin/bash

#ulimit -c unlimited
set -e

TESTFILE=testfile

[[ -e $TESTFILE ]] || dd if=/dev/zero of=$TESTFILE bs=1024 count=1048576

./slowpeng $TESTFILE ${TESTFILE}_1_enc blabla e
./slowpeng ${TESTFILE}_1_enc ${TESTFILE}_1_1_dec blabla d
./slowpeng ${TESTFILE}_1_enc ${TESTFILE}_1_2_dec blablu d
