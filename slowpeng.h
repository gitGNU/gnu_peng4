struct pengset
{
    unsigned         blksize;
    unsigned short  *perm1;
    unsigned short  *perm2;
    unsigned char   *mask;
};


struct pengpipe
{
    struct pengset **mtx;     /* 2d matrix, null terminated */
};



struct pengset *genpengset(unsigned blksize);
void execpengset(struct pengset *p, const unsigned char *buf1, unsigned char *tmpbuf, unsigned char *buf2, char encrypt);
