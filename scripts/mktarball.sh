#! /bin/bash

ARC="$1"
DIR="`basename $ARC .tar.bz2`"
shift

mkdir $DIR
tar -cf - "$@" |tar -xf - -C $DIR
tar -cjf $ARC $DIR
rm -fr $DIR

if [ "$USER" = "kj" ]
then
 gpg -u D218DF91 -b $ARC
 scp $ARC* yanestra@dl.sv.gnu.org:/releases/peng4/
fi

mv $ARC* tarball
