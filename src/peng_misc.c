/*
    PENG - A Permutation Engine
    Copyright (C) 1998-2016 by Klaus-J. Wolf
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
#include <assert.h>

#include "sysparm.h"
#include "peng_misc.h"


void *chkmalloc(uint32_t x)
{
    void *p = malloc(x);
    if(!p)
    {
        fputs("out of memory error\n", stderr);
        abort();
    }
    return p;
}



void memxor(void *dst0, const void *src0, uint32_t sz0)
{
    register uint8_t *dst = (uint8_t *)dst0;
    register const uint8_t *src = (const uint8_t *)src0;
    register uint32_t sz = sz0;
    
    while(sz--)
        *dst++ ^= *src++;
}


int kjw_memcmp(const void *abuf0, const void *bbuf0, uint32_t sz0)
{
    register const uint8_t *abuf = (const uint8_t *)abuf0;
    register const uint8_t *bbuf = (const uint8_t *)bbuf0;
    register uint32_t sz = sz0;
    
    while(sz--)
        if(*abuf++ != *bbuf++)
            return (*--abuf < *--bbuf) ? -1 : 1;
    return 0;
}


/* TODO: this is POSIX (Linux) only */
uint32_t do_padding(void *buf0, uint32_t sz0)
{
    FILE *f;
    register uint8_t *buf = (uint8_t *)buf0;
    register uint32_t sz = sz0;
    int c;

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
            *buf++ = (uint8_t) c;
        }
        fclose(f);
    }
    return 0;
}


uint32_t countconsecutivezeros(void *buf0, uint32_t sz)
{
    register uint8_t *buf = (uint8_t *)buf0;
    uint32_t res=0;
    uint32_t c=0;
    
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


static uint16_t amask16(char c)
{
    switch(c)
    {
        case '0':
            return UINT16_C(0x00ff);
        case '1':
            return UINT16_C(0xff00);
    }
    return 0;
}


static uint32_t amask32(char c)
{
    switch(c)
    {
        case '0':
            return UINT32_C(0x000000ff);
        case '1':
            return UINT32_C(0x0000ff00);
        case '2':
            return UINT32_C(0x00ff0000);
        case '3':
            return UINT32_C(0xff000000);
    }
    return 0;
}


static uint64_t amask64(char c)
{
    switch(c)
    {
        case '0':
            return UINT64_C(0x00000000000000ff);
        case '1':
            return UINT64_C(0x000000000000ff00);
        case '2':
            return UINT64_C(0x0000000000ff0000);
        case '3':
            return UINT64_C(0x00000000ff000000);
        case '4':
            return UINT64_C(0x000000ff00000000);
        case '5':
            return UINT64_C(0x0000ff0000000000);
        case '6':
            return UINT64_C(0x00ff000000000000);
        case '7':
            return UINT64_C(0xff00000000000000);
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
            return 8;
        case '2':
            return 16;
        case '3':
            return 24;
        case '4':
            return 32;
        case '5':
            return 40;
        case '6':
            return 48;
        case '7':
            return 56;
    }
    return 0;
}


static uint32_t doshift(uint32_t v, int n)
{
    if(n<0)
        return v<<(-n);
    if(n>0)
        return v>>n;
    return v;
}


#define REORDER16(f,t,i,j) j |= doshift(i&amask16(f), ashift(f)-ashift(t));


uint16_t byte_reorder16(const char *from_order, const char *to_order, uint16_t from)
{
    uint16_t res = 0;
    
    if(!strcmp(from_order,to_order))
        return from;
    
    REORDER16(from_order[0], to_order[0], from, res)
    REORDER16(from_order[1], to_order[1], from, res)

    return res;
}


#define REORDER32(f,t,i,j) j |= doshift(i&amask32(f), ashift(f)-ashift(t));


uint32_t byte_reorder32(const char *from_order, const char *to_order, uint32_t from)
{
    uint32_t res = 0;
    
    if(!strcmp(from_order,to_order))
        return from;
    
    REORDER32(from_order[0], to_order[0], from, res)
    REORDER32(from_order[1], to_order[1], from, res)
    REORDER32(from_order[2], to_order[2], from, res)
    REORDER32(from_order[3], to_order[3], from, res)

    return res;
}


#define REORDER64(f,t,i,j) j |= doshift(i&amask64(f), ashift(f)-ashift(t));


uint64_t byte_reorder64(const char *from_order, const char *to_order, uint64_t from)
{
    uint64_t res = 0;
    
    if(!strcmp(from_order,to_order))
        return from;
    
    REORDER64(from_order[0], to_order[0], from, res)
    REORDER64(from_order[1], to_order[1], from, res)
    REORDER64(from_order[2], to_order[2], from, res)
    REORDER64(from_order[3], to_order[3], from, res)
    REORDER64(from_order[4], to_order[4], from, res)
    REORDER64(from_order[5], to_order[5], from, res)
    REORDER64(from_order[6], to_order[6], from, res)
    REORDER64(from_order[7], to_order[7], from, res)

    return res;
}



void rectify32(const char *from_order, const char *to_order, void *ptr, int numbytes)
{
    uint32_t x;
    
    assert((numbytes%4)==0);
    while(numbytes>0)
    {
        x = byte_reorder32(from_order, to_order, *(uint32_t *)ptr);
        *(uint32_t *)ptr = x;
        numbytes -= sizeof(uint32_t);
        ptr = (void *)(((uint8_t *) ptr) + sizeof(uint32_t));
    }
}


/* TODO: check */
int strstrix(const char *haystack, const char *needle)
{
    int i,j;
    
    for(i=0; haystack[i]; i++)
    {
        for(j=0; haystack[i+j]==needle[j]; j++);
        if(needle[j]==0)
            return i;
    }
    return -1;
}

    
/* TODO: check */
int count_occurrences(const char *haystack, const char *needle)
{
    int i, off=0, num=0;
    
    for(;;)
    {
        i = strstrix(haystack+off, needle);
        if(i<0)
            break;
        num++; off+=i+strlen(needle);
    }
    return num;
}


/* TODO: check */
void kjw_memmove(char *buf, int f, int t, int sz)
{
    register char *pf, *pt;
    
    if(f<t)
    {
        pf=buf+f+sz-1; pt=buf+t+sz-1;
        while(sz--)
            *pt-- = *pf--;
    }
    else
        if(f>t)
        {
            pf=buf+f; pt=buf+t;
            while(sz--)
                *pt++ = *pf++;
        }
}


/* TODO: check */
int kjw_min(int a, int b)
{
    return (a<b)?a:b;
}


/* TODO: check */
void quickrepl(char *buf, const char *orig, const char *dest)
{
    int i;
    int n = strlen(buf);
    int nd = strlen(dest);
    int delta = nd-strlen(orig);
    
    for(;;)
    {
        i = strstrix(buf, orig);
        kjw_memmove(buf, i, i+delta, 1 + kjw_min(n-i, n-(i+delta)));
        memcpy(buf+i, dest, nd);
    }
}


/* TODO: check */
const char *quickrepl_dyn(const char *fmt, const char *orig, const char *dest)
{
    int total_len;
    char *buf;
    
    total_len = strlen(fmt);
    total_len += count_occurrences(orig,dest) * (strlen(dest)-strlen(orig)) + 1;
    buf = MALLOC(total_len);
    quickrepl(buf, orig, dest);
    return buf;
}

