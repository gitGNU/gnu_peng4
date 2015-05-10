
#include <stdio.h>
#include <stdlib.h>

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
    register const unsigned char *src = (unsigned char *)src0;
    register unsigned sz = sz0;
    
    while(sz--)
        *dst++ ^= *src++;
}


/* TODO: this is POSIX only */
void do_padding(void *buf0, unsigned sz0)
{
    FILE *f = fopen("/dev/urandom", "r");
    register unsigned char *buf = (unsigned char *)buf0;
    register unsigned sz = sz0;
    int c;
    
    while(sz--)
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

