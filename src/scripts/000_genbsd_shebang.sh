#! /bin/sh

HERE="`dirname $0`"

replaceshebang()
{
    BINSUBDIR=$1
    SCRIPTDIR=$2
    mkdir -p $HERE/$SCRIPTDIR
    for a in $HERE/*.sh
    do
        nn=$HERE/$SCRIPTDIR/`basename $a .sh`_bsd.sh
        sed -e 's#/bin/bash#/usr/local/bin/bash#1' <$a >$nn
        chmod 755 $nn
    done
    for a in $HERE/*.py
    do
        nn=$HERE/$SCRIPTDIR/`basename $a .py`_bsd.py
        sed -e 's#/usr/bin/python#/usr/local/bin/python#1' <$a >$nn
        chmod 755 $nn
    done
}


replaceshebang /usr/local freebsd
replaceshebang /usr/pkg netbsd
