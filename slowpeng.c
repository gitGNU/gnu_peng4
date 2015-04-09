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

#define BUFSIZE 0x400


#define QBITCOPY qbitcopy

#define QXOR(i, buf, msk) buf[i] ^= msk[i];

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


static void qbitcopy(const unsigned char *buf1, unsigned off1, unsigned char *buf2, unsigned off2)
{
    if(buf1[off1/8] & (1<<(off1&7)))
        buf2[off2/8] |= (unsigned char)(1<<(off2&7));
    else
        buf2[off2/8] &= ~(unsigned char)(1<<(off2&7));
}


struct pengset *genpengset(unsigned blksize, struct mersennetwister *mt)
{
    unsigned blksize8 = blksize*8;
    struct pengset *res = MALLOC(sizeof(struct pengset));
    char *tempflg1 = MALLOCA(blksize8);
    char *tempflg2 = MALLOCA(blksize8);
    int i,j,k;
    
    memset(tempflg1, 0, blksize8);
    memset(tempflg2, 0, blksize8);
    res->blksize = blksize;
    res->perm1 = MALLOC(blksize8);
    res->perm2 = MALLOC(blksize8);
    res->mask  = MALLOC(blksize);
    
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
            QBITCOPY(buf1, p->perm1[i], buf2, p->perm2[i]);
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
            QBITCOPY(tmpbuf, p->perm2[i], buf2, p->perm1[i]);
        }
#else
        memcpy(buf2, tmpbuf, blksize);
#endif
    }
}


int main(int argc, char **argv)
{
    int h1, h2, i, j, num=0, eflag=0;
    char *buf1 = MALLOC(BUFSIZE);
    char *buf2 = MALLOC(BUFSIZE);
    char *buf3 = MALLOC(BUFSIZE);
    struct pengset *ps;
    struct mersennetwister mt;
    
    if(argc<5)
    {
        fprintf(stderr, "usage: %s infile outfile pass e|d\n", argv[0]);
        return 1;
    }
    
    i = strlen(argv[3]);
    memset(buf1, 0, BUFSIZE);
    strncpy(buf1, argv[3], j=(i>BUFSIZE)?(BUFSIZE):i);
    mersennetwister_init_by_array(&mt, (unsigned long *)buf1, (j+3)/4);   /* TODO: fix byte order */
    memset(buf1, 0, BUFSIZE);

    eflag = !strcmp(argv[4], "e");
    
    printf("%s -%c-> %s\n", argv[1], eflag?'e':'d', argv[2]);
    
    ps = genpengset(BUFSIZE, &mt);
    
    h1 = open(argv[1], O_RDONLY);
    if(h1<0)
    {
        perror(argv[1]);
        return 99;
    }
    h2 = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if(h2<0)
    {
        perror(argv[2]);
        close(h1);
        return 99;
    }
    for(;;)
    {
        printf("block #%d\r", ++num);
        fflush(stdout);
        
        memset(buf1, 0, BUFSIZE);
        i = read(h1, buf1, BUFSIZE);
        if(i<0)
        {
            perror(argv[1]);
            return 90;
        }
        if(i<=0)
            break;
        
        if(!eflag && i<BUFSIZE)
        {
            fputs("warning: expected a full block while reading for decryption\n", stderr);
        }
        
        memset(buf2, 0, BUFSIZE);
        memset(buf3, 0, BUFSIZE);
        execpengset(ps, buf1, buf2, buf3, eflag);
        
        j = write(h2, buf3, eflag?(BUFSIZE):i);
        if(j<0)
        {
            perror(argv[2]);
            return 90;
        }
        if(i!=j)
        {
            fputs("warning: bytes read != bytes written\n", stderr);
        }
    }
    close(h1);
    close(h2);
    destroypengset(ps);
    free(buf1);
    free(buf2);
    free(buf3);

    return 0;
}
