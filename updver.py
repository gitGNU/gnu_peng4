#! /usr/bin/python2

import os,re

# const char *peng_version = "4.01.000.002";
re_ver = re.compile("_version\\s=\\s\".*?(\\d\\d\\d\\d)\";\\s*/\*\\s*CHANGEME")

try:
    os.unlink("peng.c~")
except OSError:
    pass
os.rename("peng.c","peng.c~")

f1=open("peng.c~","rt")
f2=open("peng.c","wt")
for s in f1:
    r = re_ver.search(s)
    if r:
        i = int(r.group(1))+1
        si = "%03d" % i
        s = s[:r.start(1)]+si+s[r.end(1):]
    f2.write(s)
f1.close()
f2.close()

