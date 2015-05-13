#! /usr/bin/python2

import sys

f = open("log","rt")

all=[]
triple=None

for s in f:
    
    if s.startswith("SIZE="):
        if triple:
            all.append(triple)
        triple=[int(s[5:]),None,None]
    elif "testfile_dec_1" in s:
        triple[2]=s.split()[0]
    elif "testfile" in s:
        triple[1]=s.split()[0]

f.close()

hashes=set()

print >>sys.stderr, "total", len(all)
for x in all:
    if x[1] in hashes:
        print "dupe", x[1]
        hashes.add(x[1])
    if x[1]!=x[2]:
        print "mismatch", x[0]
        if x[2] in hashes:
            print "dupe", x[2]
            hashes.add(x[2])
