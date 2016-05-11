/*
 * wolf64.c  ---  derived from adler32.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#define CHAR_BIT 8

uint32_t rotl32 (uint32_t n, unsigned int c)
{
  const unsigned int mask = (CHAR_BIT*sizeof(n)-1);

  assert ( (c<=mask) &&"rotate by type width or more");
  c &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
  return (n<<c) | (n>>( (-c)&mask ));
}

uint32_t rotr32 (uint32_t n, unsigned int c)
{
  const unsigned int mask = (CHAR_BIT*sizeof(n)-1);

  assert ( (c<=mask) &&"rotate by type width or more");
  c &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
  return (n>>c) | (n<<( (-c)&mask ));
}


#define BUFLEN 0x4000
#define MOD_ADLER 65521

#define BACKMASK32(x) ((((x)>>16) ^ (x)) & 0xffffu)

uint64_t wolf64(int fh) 
{
    uint32_t a = 0xdeed, b = 0x4134;
    uint32_t c = 0xc1f, d = 0x4a11;
    uint64_t o = 0;
    unsigned char buf[BUFLEN];
    int i, n, shiftval;
    
    for(;;)
    {
        n = read(fh, buf, BUFLEN);
        if(n<=0)
            break;
        for(i=0; i<n; i++)
        {
            a = (a + buf[i]) % MOD_ADLER;
            b = (b + a) % MOD_ADLER;
            shiftval = ((o++)%3)+1;
            c ^= buf[i];
            c = rotl32(c, shiftval);
            d = (d^c)+b;
        }
    }
    
    return (((uint64_t)BACKMASK32(d))<<48) ^ (((uint64_t)a)<<32) ^ 
           (((uint64_t)BACKMASK32(c))<<16) ^ (((uint64_t)b)); 
}


#ifdef MAIN

int main(int argc, char *argv[])
{
    int i,fh;
    
    for(i=1; i<argc; i++)
    {
        fh = open(argv[i], O_RDONLY);
        if(fh<0)
        {
            perror(argv[i]);
            continue;
        }
        printf("%016llx", wolf64(fh));
        close(fh);
        if(argc>2)
            printf("\t%s\n", argv[i]);
        else
            putchar('\n');
    }
}

#endif
