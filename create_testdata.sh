#! /bin/sh
mkdir -p testdata
dd if=/dev/urandom of=testdata/x10mb bs=16 count=1M
dd if=/dev/urandom of=testdata/x100mb bs=128 count=1M
dd if=/dev/urandom of=testdata/x1gb bs=1024 count=1M
dd if=/dev/urandom of=testdata/x10gb bs=1024 count=10M
