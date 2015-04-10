/*
 * slowpeng.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>
#include <string.h>

#include "mt19937ar.h"
#include "slowpeng.h"
#include "peng_glob.h"


#ifndef DORKY
#define DORKY 1
#endif
#ifndef SKIP_XOR
#define SKIP_XOR 1
#endif
#ifndef SKIP_PERMUT
#define SKIP_PERMUT 1
#endif

#if DEBUG
#define QBITCOPY(b1,o1,m1,b2,o2,m2) qbitcopy(b1,o1,m1,b2,o2,m2)
#else
#define QBITCOPY(b1,o1,m1,b2,o2,m2) qbitcopy(b1,o1,b2,o2)
#endif

#define QXOR(i, buf, msk) buf[i] ^= msk[i]


static unsigned char q2c(unsigned long long x)
{
    /* return (unsigned char)((x ^ (x>>8) ^ (x>>16) ^ (x>>24) ^ (x>>32) ^ (x>>40) ^ (x>>48) ^ (x>>56)) & 0xff); */
    return (unsigned char)((x ^ (x>>8) ^ (x>>15) ^ (x>>24) ^ (x>>33) ^ (x>>40) ^ (x>>48) ^ ((x>>56)+1)) & 0xff);
}


#if DEBUG
static void qbitcopy(const unsigned char *buf1, unsigned off1, unsigned max1, unsigned char *buf2, unsigned off2, unsigned max2)
{
    if(off1/8>=max1 || off2/8>=max2)
    {
        fprintf(stderr, "PANIC: %u>%u or %u>%u\n", off1/8, max1, off2/8, max2);
        abort();
    }
    
    if(buf1[off1/8] & (1U<<(off1&7)))
        buf2[off2/8] |= (unsigned char)(1U<<(off2&7));
    /*
    else
        buf2[off2/8] &= ~(unsigned char)(1U<<(off2&7));
    */
}
#else
static void qbitcopy(const unsigned char *buf1, unsigned off1, unsigned char *buf2, unsigned off2)
{
    if(buf1[off1/8] & (1U<<(off1&7)))
        buf2[off2/8] |= (unsigned char)(1U<<(off2&7));
    /*
    else
        buf2[off2/8] &= ~(unsigned char)(1U<<(off2&7));
    */
}
#endif


struct pengset *genpengset(unsigned blksize, struct mersennetwister *mt)
{
    unsigned blksize8 = blksize*8;
    struct pengset *res = MALLOC(sizeof(struct pengset));
    char *tempflg1 = MALLOCA(blksize8*sizeof(char));
    char *tempflg2 = MALLOCA(blksize8*sizeof(char));
    int i,j,k;
    
    memset(tempflg1, 0, blksize8);
    memset(tempflg2, 0, blksize8);
    res->blksize = blksize;
    res->perm1 = MALLOC(blksize8*sizeof(unsigned short));
    res->perm2 = MALLOC(blksize8*sizeof(unsigned short));
    res->mask  = MALLOC(blksize*sizeof(unsigned char));
    memset(res->perm1, 0, blksize8);
    memset(res->perm2, 0, blksize8);
    
    for(i=0; i<blksize8; i++)
    {
#if DORKY
        j = mersennetwister_genrand_int32(mt) % blksize8;
        /* this is dorky, but it speeds things up a lot */
        while(tempflg1[j])
            j = (j+1)%blksize8;
        k = mersennetwister_genrand_int32(mt) % blksize8;
        /* this is dorky, but it speeds things up a lot */
        while(tempflg2[k])
            k = (k+1)%blksize8;
#else
        do
            j = mersennetwister_genrand_int32(mt) % blksize8;
        while(tempflg1[j]);
        do
            k = mersennetwister_genrand_int32(mt) % blksize8;
        while(tempflg2[k]);
#endif
        tempflg1[j] = 1;
        tempflg2[k] = 1;
        res->perm1[i] = j;
        res->perm2[i] = k;
    }
    
    FREEA(tempflg1);
    FREEA(tempflg2);
    
    for(i=0; i<blksize; i++)
    {
        res->mask[i] = q2c(mersennetwister_genrand_int32(mt));
    }
    
    return res;
}


struct pengpipe *genpengpipe(unsigned blksize, unsigned rounds, unsigned variations, struct mersennetwister *mt)
{
    int i,j;
    struct pengpipe *res = MALLOC(sizeof(struct pengpipe));
    
    res->blksize = blksize;
    res->rounds = rounds;
    res->variations = variations;
    res->mtx = MALLOC(variations * sizeof(struct pengset **));
    for(i=0; i<variations; i++)
    {
        res->mtx[i] = MALLOC(rounds * sizeof(struct pengset *));
    }
    for(i=0; i<variations; i++)
    {
        for(j=0; j<rounds; j++)
        {
            res->mtx[i][j] = genpengset(blksize, mt);
        }
    }
    return res;
}


void destroypengset(struct pengset *p)
{
    FREE(p->perm1);
    FREE(p->perm2);
    FREE(p->mask);
    FREE(p);
}


void destroypengpipe(struct pengpipe *p)
{
    int i,j;
    
    for(i=0; i<p->variations; i++)
    {
        for(j=0; j<p->rounds; j++)
        {
            destroypengset(p->mtx[i][j]);
        }
    }
    for(i=0; i<p->variations; i++)
    {
        FREE(p->mtx[i]);
    }
    FREE(p->mtx);
    FREE(p);
}


void execpengset(struct pengset *p, const unsigned char *buf1, unsigned char *tmpbuf, unsigned char *buf2, char encrypt)
{
    unsigned blksize = p->blksize;
    unsigned blksize8 = blksize*8;
    int i;
    
    if(encrypt)
    {
#if !SKIP_PERMUT
        for(i=0; i<blksize8; i++)
        {
            QBITCOPY(buf1, p->perm1[i], blksize8, buf2, p->perm2[i], blksize8);
        }
#else
        memcpy(buf2, buf1, blksize);
#endif
#if !SKIP_XOR
        for(i=0; i<blksize; i++)
        {
            QXOR(i, buf2, p->mask);
        }
#endif
    }
    else
    {
        memcpy(tmpbuf, buf1, blksize);
#if !SKIP_XOR
        for(i=0; i<blksize; i++)
        {
            QXOR(i, tmpbuf, p->mask);
        }
#endif
#if !SKIP_PERMUT
        for(i=0; i<blksize8; i++)
        {
            QBITCOPY(tmpbuf, p->perm2[i], blksize8, buf2, p->perm1[i], blksize8);
        }
#else
        memcpy(buf2, tmpbuf, blksize);
#endif
    }
}


void execpengpipe(struct pengpipe *p, unsigned char *buf1, unsigned char *tmpbuf, unsigned char *buf2, char encrypt)
{
    int i,j;
    unsigned off;
    
    for(i=0; i<p->variations; i++)
    {
        off = i*p->blksize;
        for(j=0; j<p->rounds; j++)
        {
            execpengset(p->mtx[i][j], buf1+off, tmpbuf, buf2+off, encrypt);
            /* copy buf2 back to buf1 */
            if(j<p->rounds-1)
            {
                memcpy(buf1+off, buf2+off, p->blksize);
            }
        }
    }
}


unsigned long getbufsize(struct pengpipe *p)
{
    return (unsigned long)p->blksize * p->variations;
}
