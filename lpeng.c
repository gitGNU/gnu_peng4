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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sysparm.h"
#include "whirlpool.h"
#include "mt19937ar.h"
#include "peng_misc.h"
#include "peng_ref.h"
#include "sha2.h"
#include "lpeng.h"


/* global */ int verbosity = 0;


const uint32_t eof_magic[] = { 0x1a68b01ful, 0x4a11c153ul, 0x436621e9ul, 0xe710ffb4ul };


#define COMBINED_DIGEST_SIZE (WHIRLPOOL_DIGESTBYTES+SHA512_DIGEST_SIZE)


/* attention ! passphrase will be erased ! */
void peng_cmd_prep(struct peng_cmd_environment *pce, uint32_t blksize, uint32_t rounds, uint32_t variations, char *passphrase, int eflag)
{
    struct whirlpool wp;
    sha512_ctx sha512;
    uint8_t digest[WHIRLPOOL_DIGESTBYTES];
    uint8_t digest2[SHA512_DIGEST_SIZE];
    uint8_t combined[COMBINED_DIGEST_SIZE];
    
    whirlpool_init(&wp);
    whirlpool_add(&wp, (uint8_t *) passphrase, strlen(passphrase)*8);
    whirlpool_finalize(&wp, digest);
    
    sha512_init(&sha512);
    sha512_update(&sha512, (uint8_t *) passphrase, strlen(passphrase));
    sha512_final(&sha512, digest2);
    
    memcpy(combined, digest, WHIRLPOOL_DIGESTBYTES);
    memcpy(combined+WHIRLPOOL_DIGESTBYTES, digest2, SHA512_DIGEST_SIZE);
    
    rectify(SYSTEM_BYTEORDER, TARGET_BYTEORDER, combined, COMBINED_DIGEST_SIZE);
    
    mersennetwister_init_by_array(&pce->mt, (uint32_t *)combined, COMBINED_DIGEST_SIZE/sizeof(uint32_t));  /* TODO byte order, packing */
    
    pce->pp = genpengpipe(blksize, rounds, variations, &pce->mt);
    pce->blksize = blksize;
    pce->bufsize = getbufsize(pce->pp);
    pce->eflag = eflag?1:0;
    
    pce->buf1 = MALLOC(pce->bufsize);
    pce->buf2 = MALLOC(pce->bufsize);
    pce->buf3 = MALLOC(pce->bufsize);
    
    memset(passphrase, 0, strlen(passphrase));
    memset(&wp, 0, sizeof wp);
    memset(digest, 0, WHIRLPOOL_DIGESTBYTES);
}


int peng_cmd_process(struct peng_cmd_environment *pce, const char *infn, int inh, uint64_t total, const char *outfn, int outh, char multithreading, char min_locrr_seq_len)
{
    int i,j,k,r,z;
    int guess_eof = 0, truncate_at = 0;
    uint32_t num=0, padding_remaining=0;
    uint64_t pos=0;
    
    for(;;)
    {
        if(verbosity>1)
        {
            printf("block #%u\r", ++num);
            fflush(stdout);
        }
        
        /* memset(pce->buf1, 0, pce->bufsize); */
        i = read(inh, pce->buf1, pce->bufsize);
        if(i<0)
        {
            perror(infn);
            return -1;
        }
        
        if(i<=0)
            break;
        
        pos += i;
        if(total>0 && pos>=total)
            guess_eof = 1;
        
        if(pce->eflag && i<pce->bufsize)
        {
            /* Pad buffer with random data and mark EOF.
             * special case: the input file matches the block size exactly; then:
             * do NOT use a EOF magic
             */
            padding_remaining = do_padding(pce->buf1+i, pce->bufsize-i, eof_magic, sizeof eof_magic / sizeof eof_magic[0], 0);
        }
        
        if(!pce->eflag && i<pce->bufsize)
        {
            fprintf(stderr, "warning: %s: expected a full block while reading for decryption\n", outfn);
        }
        
        /* memset(pce->buf2, 0, pce->bufsize); */
        /* memset(pce->buf3, 0, pce->bufsize); */
        /* execpengset(ps, buf1, buf2, buf3, eflag); */
        execpengpipe(pce->pp, pce->buf1, pce->buf2, pce->buf3, pce->eflag, multithreading);
        
        k = pce->eflag?(pce->bufsize):i;
        
        if(!pce->eflag && guess_eof)
        {
            /* Find the EOF marker.
             */
            z = locrr(pce->buf3, k, eof_magic, sizeof eof_magic / sizeof eof_magic[0], min_locrr_seq_len);
            if(z>=-9999)   /* legal value */
            {
                if(verbosity>1)
                    fprintf(stderr, "applying guess_eof at z=%d\n", z);
                if(z>=0)
                    k = z;
                else
                {
                    /* z is negative and valid, that means the start of the marker was in the previous block */
                    k = 0;
                    truncate_at = z;
                    break;
                }
            }
        }
        
        j = write(outh, pce->buf3, k);
        if(j<0)
        {
            perror(outfn);
            return -1;
        }
        if(k!=j)
        {
            fprintf(stderr, "warning: %s: bytes buffered not equal bytes written\n", outfn);
        }
        if(guess_eof)
            break;    /* this is to avoid some tricky problems (with growing files) */
    }
    /* if(!pce->eflag && guess_eof && k==0 && z<0 && z>=-9999) */
    if(truncate_at)
    {
        pos = lseek(outh, 0, SEEK_CUR);
        lseek(outh, 0, SEEK_SET);
        r = ftruncate(outh, pos-truncate_at);
        if(r)
        {
            perror(outfn);
            return -1;
        }
    }
    else
    if(padding_remaining)  /* encrypting */
    {
        do_padding(pce->buf1, pce->bufsize, eof_magic, sizeof eof_magic / sizeof eof_magic[0], padding_remaining);
        execpengpipe(pce->pp, pce->buf1, pce->buf2, pce->buf3, pce->eflag, multithreading);
        k = pce->bufsize;
        j = write(outh, pce->buf3, k);
        if(j<0)
        {
            perror(outfn);
            return -1;
        }
        if(k!=j)
        {
            fprintf(stderr, "warning: %s: bytes buffered not equal bytes written (fin)\n", outfn);
        }
    }
    
    return 0;
}


void peng_cmd_unprep(struct peng_cmd_environment *pce)
{
    destroypengpipe(pce->pp);
    FREE(pce->buf1);
    FREE(pce->buf2);
    FREE(pce->buf3);
}


