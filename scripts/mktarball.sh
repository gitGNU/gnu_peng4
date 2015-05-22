#! /bin/bash

ARC="$1"
DIR="`basename $ARC .tar.bz2`"
shift

GPGKEY=0D1099BA

mkdir $DIR
tar -cf - "$@" |tar -xf - -C $DIR
tar -cjf $ARC $DIR
rm -fr $DIR

if [ "$USER" = "kj" ]
then
    if gpg -u $GPGKEY -b $ARC
    then
        scp $ARC* yanestra@dl.sv.gnu.org:/releases/peng4/
    else
        echo '*****' $ARC is prepared
    fi
    exit 0
fi

mv $ARC* tarball
