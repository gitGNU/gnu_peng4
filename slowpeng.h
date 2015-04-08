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



