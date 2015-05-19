/*
    PENG - A Permutation Engine
    Copyright (C) 1998-2015 by Klaus-J. Wolf
                               yanestra !at! lab6 !dot! seismic !dot! de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or   
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>
#include <string.h>
#include <pthread.h>

#include "mt19937ar.h"
#include "peng_ref.h"
#include "peng_glob.h"


#ifndef DORKINESS
#define DORKINESS 0   /* set 0,1,2 */
#endif
#ifndef SKIP_XOR
#define SKIP_XOR 0
#endif
#ifndef SKIP_PERMUT
#define SKIP_PERMUT 0
#endif

#if DEBUG
#define QBITCOPY(b1,o1,m1,b2,o2,m2) qbitcopy(b1,o1,m1,b2,o2,m2)
#else
#define QBITCOPY(b1,o1,m1,b2,o2,m2) { if(b1[o1/8] & (1U<<(o1&7)))     b2[o2/8] |= (unsigned char)(1U<<(o2&7)); }
#endif


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
    res->mask1  = MALLOC(blksize*sizeof(unsigned char));
#if USE_MODE_XPX
    res->mask2  = MALLOC(blksize*sizeof(unsigned char));
#endif
    memset(res->perm1, 0, blksize8);
    memset(res->perm2, 0, blksize8);
    
    for(i=0; i<blksize8; i++)
    {
#if DORKINESS>1
        j = mersennetwister_genrand_int32(mt) % blksize8;
        /* this is dorky, but it speeds things up a lot */
        while(tempflg1[j])
            j = (j+1)%blksize8;
        k = mersennetwister_genrand_int32(mt) % blksize8;
        /* this is dorky, but it speeds things up a lot */
        while(tempflg2[k])
            k = (k+1)%blksize8;
#elif DORKINESS==1
        do
            j = mersennetwister_genrand_int32(mt) % blksize8;
        while(tempflg1[j]);
        do
            k = mersennetwister_genrand_int32(mt) % blksize8;
        while(tempflg2[k]);
#else
        /* TODO this is even better, but can be really slow... */
        do
            j = mersennetwister_genrand_int32_strong(mt, blksize8);
        while(tempflg1[j]);
        do
            k = mersennetwister_genrand_int32_strong(mt, blksize8);
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
        res->mask1[i] = q2c(mersennetwister_genrand_int32(mt));
#if USE_MODE_XPX
        res->mask2[i] = q2c(mersennetwister_genrand_int32(mt));
#endif
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
            if(verbosity>1)
            {
                printf("generating variation=%d, round=%d  \r", i, j);
                fflush(stdout);
            }
            res->mtx[i][j] = genpengset(blksize, mt);
        }
    }
#if USE_MODE_CBC
    res->iv = MALLOC(blksize);
    for(i=0; i<blksize; i++)
    {
        res->iv[i] = q2c(mersennetwister_genrand_int32(mt));
    }
#endif
    fputs("\n", stdout);
    return res;
}


void destroypengset(struct pengset *p)
{
    FREE(p->perm1);
    FREE(p->perm2);
    FREE(p->mask1);
#if USE_MODE_XPX
    FREE(p->mask2);
#endif
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
#if USE_MODE_CBC
    FREE(p->iv);
#endif
    FREE(p);
}


void execpengset(struct pengset *p, const unsigned char *buf1, unsigned char *tmpbuf, unsigned char *buf2, char encrypt)
{
    unsigned blksize = p->blksize;
    unsigned blksize8 = blksize*8;
    int i;
    
    if(encrypt)
    {
        memcpy(tmpbuf, buf1, blksize);
#if !SKIP_XOR && USE_MODE_XPX
        memxor(tmpbuf, p->mask2, blksize);
#endif
#if !SKIP_PERMUT
        memset(buf2, 0, blksize);
        for(i=0; i<blksize8; i++)
        {
            QBITCOPY(tmpbuf, p->perm1[i], blksize8, buf2, p->perm2[i], blksize8);
        }
#else
        memcpy(buf2, tmpbuf, blksize);
#endif
#if !SKIP_XOR
        memxor(buf2, p->mask1, blksize);
#endif
    }
    else
    {
        memcpy(tmpbuf, buf1, blksize);
#if !SKIP_XOR
        memxor(tmpbuf, p->mask1, blksize);
#endif
#if !SKIP_PERMUT
        memset(buf2, 0, blksize);
        for(i=0; i<blksize8; i++)
        {
            QBITCOPY(tmpbuf, p->perm2[i], blksize8, buf2, p->perm1[i], blksize8);
        }
#else
        memcpy(buf2, tmpbuf, blksize);
#endif
#if !SKIP_XOR && USE_MODE_XPX
        memxor(buf2, p->mask2, blksize);
#endif
    }
}


struct epp_thr_context
{
    struct pengset    **mtx;
    unsigned            rounds;
    unsigned            blksize;
    unsigned char      *buf1;
    unsigned char      *tmpbuf;
    unsigned char      *buf2;
    char encrypt;
#if USE_MODE_CBC
    unsigned char      *iv;
#endif
};


static void *epp_thr(void *param)
{
    int j;
    struct epp_thr_context *c = (struct epp_thr_context *) param;
    
#if USE_MODE_CBC
    unsigned char *lastbuf = alloca(c->blksize);
    unsigned char *lastbuftmp = alloca(c->blksize);
    
    memcpy(lastbuf, c->iv, c->blksize);
#endif

#if USE_MODE_CBC
    if(c->encrypt)
    {
        memxor(c->buf1, lastbuf, c->blksize);
    }
    else
    {
        memcpy(lastbuftmp, c->buf1, c->blksize);
    }
#endif
    if(c->encrypt)
        for(j=0; j<c->rounds; j++)
        {
            execpengset(c->mtx[j], c->buf1, c->tmpbuf, c->buf2, c->encrypt);
            /* copy buf2 back to buf1 */
            if(j<c->rounds-1)
            {
                memcpy(c->buf1, c->buf2, c->blksize);
            }
        }
    else
        for(j=c->rounds-1; j>=0; j--)
        {
            execpengset(c->mtx[j], c->buf1, c->tmpbuf, c->buf2, c->encrypt);
            /* copy buf2 back to buf1 */
            if(j>0)
            {
                memcpy(c->buf1, c->buf2, c->blksize);
            }
        }
#if USE_MODE_CBC
    if(!c->encrypt)
    {
        memxor(c->buf2, lastbuf, c->blksize);
        memcpy(lastbuf, lastbuftmp, c->blksize);
    }
    else
    {
        memcpy(lastbuf, c->buf2, c->blksize);
    }
#endif
    return NULL;
}


void execpengpipe(struct pengpipe *p, unsigned char *buf1, unsigned char *tmpbuf, unsigned char *buf2, char encrypt, char threads_flag)
{
    int i,r;
    unsigned off;
    struct epp_thr_context *ctx;
    pthread_t *pthr = alloca(p->variations*sizeof(pthread_t));
    
    /* alloca() shouldn't work here */
    ctx = MALLOC(p->variations*sizeof(struct epp_thr_context));
    memset(ctx, 0, p->variations*sizeof(struct epp_thr_context));
    
    for(i=0; i<p->variations; i++)
    {
        off = i*p->blksize;
        
        ctx[i].mtx = p->mtx[i];
        ctx[i].rounds = p->rounds;
        ctx[i].blksize = p->blksize;
        ctx[i].buf1 = buf1+off;
        ctx[i].tmpbuf = tmpbuf+off;
        ctx[i].buf2 = buf2+off;
        ctx[i].encrypt = encrypt;
#if USE_MODE_CBC
        ctx[i].iv = p->iv;
#endif
        
        if(!threads_flag)
        {
            epp_thr(&ctx[i]);
        }
        else
        {
            r = pthread_create(&pthr[i], NULL, epp_thr, &ctx[i]);
            if(r)
            {
                perror("thread creation");
                abort();
            }
        }
    }
    if(threads_flag)
        for(i=0; i<p->variations; i++)
        {
            r = pthread_join(pthr[i], NULL);
            if(r)
            {
                perror("thread joining");
                abort();
            }
        }
    FREE(ctx);
}


unsigned long getbufsize(struct pengpipe *p)
{
    return (unsigned long)p->blksize * p->variations;
}
