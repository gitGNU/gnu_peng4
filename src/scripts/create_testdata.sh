#! /bin/bash
mkdir -p testdata
if [[ ! -e testdata/h_16mib ]]
then
 dd if=/dev/urandom of=testdata/h_16mib count=16 bs=1M
 dd if=/dev/urandom of=testdata/h_128mib count=128 bs=1M
 dd if=/dev/urandom of=testdata/h_1gib count=1K bs=1M
 dd if=/dev/urandom of=testdata/h_16gib count=16K bs=1M
fi

if [[ ! -e testdata/c_sources ]]
then
 KERNEL=~/Downloads/linux-4.4.30.tar.xz
 TMPDIR=/var/tmp/$$
 mkdir $TMPDIR
 tar -xJf $KERNEL -C $TMPDIR
 find $TMPDIR -name "*.c" -o -name "*.h" -print |sort |xargs cat >testdata/c_sources
 rm -fr $TMPDIR
fi

if [[ ! -e testdata/c_64mib ]]
then
 dd if=testdata/c_sources of=testdata/c_16mib count=16 bs=1M
 dd if=testdata/c_sources of=testdata/c_64mib count=64 bs=1M
fi
