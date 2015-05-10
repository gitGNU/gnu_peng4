#! /bin/sh
mkdir -p testdata
dd if=/dev/urandom of=testdata/x16mib count=16 bs=1M
dd if=/dev/urandom of=testdata/x128mib count=128 bs=1M
dd if=/dev/urandom of=testdata/x1gib count=1K bs=1M
dd if=/dev/urandom of=testdata/x16gib count=16K bs=1M
