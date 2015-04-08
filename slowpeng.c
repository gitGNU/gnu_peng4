/*
 * slowpeng.c
 */


#include <stdlib.h>

#include "mt19937-64.h"
#include "slowpeng.h"


#define QBITCOPY qbitcopy

#define QXOR(i, buf, msk) buf[i] ^= msk;

#define MALLOC chkmalloc



static void chkmalloc(unsigned x)
{
    void *p = malloc(x);
    if(!p)
        abort();
}


static void qbitcopy(const unsigned char *buf1, unsigned off1, unsigned char *buf2, unsigned off2)
{
    if(buf1[off1/8] & (1<<(off1&7)))
        buf2[off2/8] |= (1<<(off2&7));
    else
        buf2[off2/8] &= ~(1<<(off2&7));
}


struct pengset *genpengset(unsigned blksize)
{
    unsigned blksize8 = blksize*8;
    struct pengset *res = MALLOC(sizeof(struct pengset));
    char *tempflg1 = MALLOC(blksize8);
    char *tempflg2 = MALLOC(blksize8);
    int i,j,k;
    
    memset(tempflg1, 0, blksize8);
    memset(tempflg2, 0, blksize8);
    res->blksize = blksize;
    res->perm1 = MALLOC(blksize8);
    res->perm2 = MALLOC(blksize8);
    res->mask  = MALLOC(blksize);
    
    for(i=0; i<blksize8; i++)
    {
        do
        {
            j = genrand64_int64() % blksize8;
        }
         while(tempflg1[j]);
        do
        {
            k = genrand64_int64() % blksize8;
        }
         while(tempflg2[k]);
        res->perm1[i] = j;
        res->perm2[i] = k;
    }
    for(i=0; i<blksize; i++)
    {
        res->mask[i] = (unsigned char)(genrand64_int64() & 0xff);
    }
    
    return res;
}


void execpengset(struct pengset *p, const unsigned char *buf1, unsigned char *tmpbuf, unsigned char *buf2, char encrypt)
{
    unsigned blksize = p->blksize;
    unsigned blksize8 = blksize*8;
    int i,j,k;
    
    if(encrypt)
    {
        for(i=0; i<blksize8; i++)
        {
            QBITCOPY(buf1, res->perm1[i], buf2, res->perm2[i])
        }
        for(i=0; i<blksize; i++)
        {
            QXOR(i, buf2, res->mask)
        }
    }
    else
    {
        memcpy(tmpbuf, buf1, blksize);
        for(i=0; i<blksize; i++)
        {
            QXOR(i, tmpbuf, res->mask)
        }
        for(i=0; i<blksize8; i++)
        {
            QBITCOPY(tmpbuf, res->perm2[i], buf2, res->perm1[i])
        }
    }
}


int main(int argc, char **argv)
{
    
}
