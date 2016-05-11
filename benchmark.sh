#! /bin/bash

FN=testfile
LOG=log
PP=mHnGbT71Qaaaa-7

rm -f $LOG

[[ -e $FN ]] || dd if=/dev/urandom of=$FN bs=65536 count=1024

for a in L M H X
do
for b in L M H X
do
for c in L M H X
do
   O=${a}${b}${c}
   echo O=$O >>$LOG 
   /usr/bin/time src/peng -v -n -m -O $O -P $PP $FN >>$LOG 2>&1
   src/peng -n -d -m -O $O -P $PP $FN.enc
   if cmp -s $FN $FN.enc.dec
   then
       echo COMPARISON OK >>$LOG
   else
       echo COMPARISON FAILED >>$LOG
   fi
   echo >>$LOG
done
done
done

rm -f $FN $FN.enc $FN.enc.dec
