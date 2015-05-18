/*
    PENG - A Permutation Engine
    Copyright (C) 1998-2015 by Klaus-J. Wolf

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
unsigned do_padding(void *buf0, unsigned sz0, const unsigned long *marker, unsigned nmarker, unsigned marker_byteoffset)
{
    FILE *f;
    register unsigned char *buf = (unsigned char *)buf0;
    register unsigned sz = sz0;
    int c;
    unsigned i, r=0;

    if(marker && nmarker)
    {
        i = nmarker*sizeof(unsigned long) - marker_byteoffset;
        if(sz<i)
        {
            r = i - sz;
            i = sz;
        }
        memcpy(buf, ((unsigned char *)marker)+marker_byteoffset, i);
        
        sz-=i;
        buf+=i;
    }
    
    if(sz>0)
    {
        f = fopen("/dev/urandom", "r");
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
    return r;
}


/* TODO: byte order */
int locrr(void *buf, unsigned sz, const unsigned long *marker, unsigned nmarker, int minmatch)
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
    /* now we try to find tails at pos=0 */
    n=1;
    for(n=1; nmarker-n>minmatch; n++)
    {
        if(!mymemcmp(buf, marker+n, nmarker-n))
        {
            return -n;
        }
    }
    return -10000;  /* sigh, some number implausibly large and negative */
}


unsigned countconsecutivezeros(void *buf0, unsigned sz)
{
    register unsigned char *buf = (unsigned char *)buf0;
    unsigned res=0;
    unsigned c=0;
    
    while(sz--)
    {
        if(!*buf++)
            c++;
        else
            if(c>res)
                res=c;
    }
    if(c>res)
        res=c;
    return res;
}
