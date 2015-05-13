#! /bin/bash

ARC="$1"
DIR="`basename $ARC .tar.bz2`"
shift

mkdir $DIR
tar -cf - "$@" |tar -xf - -C $DIR
tar -cjf $ARC $DIR
rm -fr $DIR
