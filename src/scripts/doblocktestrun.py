#! /usr/bin/python3

import os, os.path, subprocess

BLKSIZE = 2048
BASEDIR = "testdata_block"

PW = "blablaBLOEBLOE"

minsz = BLKSIZE*2
maxsz = minsz+BLKSIZE+100

os.makedirs(BASEDIR, exist_ok=True)

ffns=[]

n=1
for i in range(minsz, maxsz+1):
    fn=os.path.join(BASEDIR, "BLK%05d"%n)
    n+=1
    f=open(fn, "wb")
    b=os.urandom(i)
    f.write(b)
    f.close()
    ffns.append(fn)

for fn in ffns:
    #$PROXY ./peng -m -n -O $PARM $V -P blablaBLOEBLOE ${TESTFILE}_${ID}
    subprocess.check_call(["./peng", "-m", "-n", "-O", "%d,3,3"%BLKSIZE, "-vv", "-P", PW, fn])
    #$PROXY ./peng -m -R -d -O $PARM $V -P blablaBLOEBLOE ${TESTFILE}_${ID}_dec_1
    subprocess.check_call(["./peng", "-m", "-d", "-n", "-vv", "-P", PW, fn+".enc"])
