#! /bin/sh

HERE="`dirname $0`"

for a in $HERE/*.sh
do
  if echo $a |grep bsd >/dev/null
  then
    true
  else
    nn=$HERE/`basename $a .sh`_bsd.sh
    sed -e 's#/bin/bash#/usr/local/bin/bash#1' <$a >$nn
    chmod 755 $nn
  fi
done
for a in $HERE/*.py
do
  if echo $a |grep bsd >/dev/null
  then
    true
  else
    nn=$HERE/`basename $a .py`_bsd.py
    sed -e 's#/usr/bin/python#/usr/local/bin/python#1' <$a >$nn
    chmod 755 $nn
  fi
done
