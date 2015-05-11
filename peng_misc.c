
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "peng_glob.h"


void *chkmalloc(unsigned x)
{
    void *p = malloc(x);
    if(!p)
    {
        fputs("out of memory error\n", stderr);
        abort();
    }
    return p;
}



void memxor(void *dst0, const void *src0, unsigned sz0)
{
    register unsigned char *dst = (unsigned char *)dst0;
    register const unsigned char *src = (const unsigned char *)src0;
    register unsigned sz = sz0;
    
    while(sz--)
        *dst++ ^= *src++;
}


int mymemcmp(const void *abuf0, const void *bbuf0, unsigned sz0)
{
    register const unsigned char *abuf = (const unsigned char *)abuf0;
    register const unsigned char *bbuf = (const unsigned char *)bbuf0;
    register unsigned sz = sz0;
    
    while(sz--)
        if(*abuf++ != *bbuf++)
            return (*--abuf < *--bbuf) ? -1 : 1;
    return 0;
}


/* TODO: this is POSIX only */
/* TODO: byte order */
void do_padding(void *buf0, unsigned sz0, const unsigned long *marker, unsigned nmarker)
{
    FILE *f = fopen("/dev/urandom", "r");
    register unsigned char *buf = (unsigned char *)buf0;
    register unsigned sz = sz0;
    int c;
    unsigned i;

    if(marker && nmarker)
    {
        i = nmarker*sizeof(unsigned long);
        if(sz<i)
            i = sz;
        memcpy(buf, marker, i);
        
        sz-=i;
        buf+=i;
    }
    
    while(sz-->0)
    {
        c = fgetc(f);
        if(c<0)
        {
            perror("/dev/urandom");
            abort();
        }
        *buf++ = (unsigned char) c;
    }
    fclose(f);
}


/* TODO: byte order */
unsigned locrr(void *buf, unsigned sz, const unsigned long *marker, unsigned nmarker, int minmatch)
{
    int pos = (int)sz-minmatch, n;
    
    nmarker *= sizeof(unsigned long);
    for(; pos>=0; pos--)
    {
        n = sz-pos;
        if(n>nmarker)
            n = nmarker;
        if(!mymemcmp(buf+pos, marker, n))
            return pos;
    }
    return -1;
}

