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


#ifndef DORKY
#define DORKY 1
#endif
#ifndef SKIP_XOR
#define SKIP_XOR 1
#endif
#ifndef SKIP_PERMUT
#define SKIP_PERMUT 1
#endif

#ifndef BUFSIZE
#define BUFSIZE 0x400
#endif

#if DEBUG
#define QBITCOPY(b1,o1,m1,b2,o2,m2) qbitcopy(b1,o1,m1,b2,o2,m2)
#else
#define QBITCOPY(b1,o1,m1,b2,o2,m2) qbitcopy(b1,o1,b2,o2)
#endif

#define QXOR(i, buf, msk) buf[i] ^= msk[i]

#define MALLOC chkmalloc
#define MALLOCA alloca


static unsigned char q2c(unsigned long long x)
{
    /* return (unsigned char)((x ^ (x>>8) ^ (x>>16) ^ (x>>24) ^ (x>>32) ^ (x>>40) ^ (x>>48) ^ (x>>56)) & 0xff); */
    return (unsigned char)((x ^ (x>>8) ^ (x>>15) ^ (x>>24) ^ (x>>33) ^ (x>>40) ^ (x>>48) ^ ((x>>56)+1)) & 0xff);
}


static void *chkmalloc(unsigned x)
{
    void *p = malloc(x);
    if(!p)
    {
        fputs("out of memory error\n", stderr);
        abort();
    }
    return p;
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
    /*
    free(tempflg1);
    free(tempflg2);
    */
    for(i=0; i<blksize; i++)
    {
        res->mask[i] = q2c(mersennetwister_genrand_int32(mt));
    }
    
    return res;
}


void destroypengset(struct pengset *p)
{
    free(p->perm1);
    free(p->perm2);
    free(p->mask);
    free(p);
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
