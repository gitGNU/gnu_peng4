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
#include <string.h>

#include "peng_misc.h"


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


static unsigned long amask(char c)
{
    switch(c)
    {
        case '0':
            return 0x0000000ful;
        case '1':
            return 0x000000f0ul;
        case '2':
            return 0x00000f00ul;
        case '3':
            return 0x0000f000ul;
        case '4':
            return 0x000f0000ul;
        case '5':
            return 0x00f00000ul;
        case '6':
            return 0x0f000000ul;
        case '7':
            return 0xf0000000ul;
    }
    return 0;
}


static int ashift(char c)
{
    switch(c)
    {
        case '0':
            return 0;
        case '1':
            return 4;
        case '2':
            return 8;
        case '3':
            return 12;
        case '4':
            return 16;
        case '5':
            return 20;
        case '6':
            return 24;
        case '7':
            return 28;
    }
    return 0;
}


static unsigned long doshift(unsigned long v, int n)
{
    if(n<0)
        return v<<(-n);
    if(n>0)
        return v>>n;
    return v;
}


#define REORDER(f,t,i,j) j |= doshift(i&amask(f), ashift(f)-ashift(t));


unsigned long byte_reorder(const char *from_order, const char *to_order, unsigned long from, int bytes)
{
    unsigned long res = 0;
    
    if(!strcmp(from_order,to_order))
        return from;
    
    if(bytes>0) REORDER(from_order[0], to_order[0], from, res)
    if(bytes>1)  REORDER(from_order[1], to_order[1], from, res)
    if(bytes>2)  REORDER(from_order[2], to_order[2], from, res)
    if(bytes>3)  REORDER(from_order[3], to_order[3], from, res)
    /*
    if(bytes>3)  REORDER(from_order[4], to_order[4], from, res)
    if(bytes>4)  REORDER(from_order[5], to_order[5], from, res)
    if(bytes>5)  REORDER(from_order[6], to_order[6], from, res)
    if(bytes>6)  REORDER(from_order[7], to_order[7], from, res)
    */

    printf("rectify: [%d] %08lx -> %08lx\n", sizeof(unsigned long), from, res);
    
    return res;
}



void rectify(const char *from_order, const char *to_order, void *ptr, int numbytes)
{
    unsigned long x;
    
    while(numbytes>0)
    {
        x = byte_reorder(from_order, to_order, *(unsigned long *)ptr, numbytes);
        *(unsigned long *)ptr = x;
        numbytes -= sizeof(unsigned long);
        ptr = (void *)(((unsigned char *) ptr) + sizeof(unsigned long));
    }
}

