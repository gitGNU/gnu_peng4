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
#include "wolf64.h"


/* global */ int verbosity = 0;
/* global */ int debugmask = 0;

static int init_done = 0;


#define COMBINED_DIGEST_SIZE (WHIRLPOOL_DIGESTBYTES+SHA512_DIGEST_SIZE)


void peng_unit_prep(void)
{
    if(init_done)
        return;
    init_done=1;
    /* ... */
}


/* attention ! passphrase will be erased ! */
void peng_cmd_prep(struct peng_cmd_environment *pce, uint32_t blksize, uint32_t rounds, uint32_t variations, char *passphrase, int eflag)
{
    struct whirlpool wp;
    sha512_ctx sha512;
    uint8_t digest[WHIRLPOOL_DIGESTBYTES];
    uint8_t digest2[SHA512_DIGEST_SIZE];
    uint8_t combined[COMBINED_DIGEST_SIZE];
    
    peng_unit_prep();
    
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


struct header
  {
      uint32_t        headerlen;
      uint64_t        totalsize;
      uint64_t        cksum;
  };

int peng_cmd_process(struct peng_cmd_environment *pce, const char *infn, int inh, uint64_t total, const char *outfn, int outh, char multithreading, char min_locrr_seq_len)
{
    int i,j,k,r;
    int firstblock;
    uint32_t num=0, off=0;
    uint64_t pos=0, cksum;
    struct header h;
    
    memset(&h, 0, sizeof h);
    /* DEBUG_TIMING(1, "peng_cmd_process_0") */
    
    if(pce->eflag)
    {
        lseek(inh, 0, SEEK_SET);
        h.cksum = wolf64(inh);
        h.totalsize = lseek(inh, 0, SEEK_END);   /* it is already at the end... */
        h.headerlen = sizeof h; 
        lseek(inh, 0, SEEK_SET);
        
        h.headerlen = byte_reorder32(SYSTEM_BYTEORDER, TARGET_BYTEORDER, h.headerlen, 4);
        h.totalsize = byte_reorder64(SYSTEM_BYTEORDER, TARGET_BYTEORDER, h.totalsize, 8);
        h.cksum = byte_reorder64(SYSTEM_BYTEORDER, TARGET_BYTEORDER, h.cksum, 8);
        
        memcpy(pce->buf1, &h, sizeof h);
        off = sizeof h;
    }

    for(firstblock=1; ; firstblock=0)
    {
        if(verbosity>1)
        {
            printf("block #%u\r", ++num);
            fflush(stdout);
        }
        
        /* memset(pce->buf1, 0, pce->bufsize); */
        i = read(inh, pce->buf1+off, pce->bufsize-off);
        if(i<0)
        {
            perror(infn);
            return -1;
        }
        
        i += off;
        if(i<=0)
            break;
        
        off = 0;
        
        if(pce->eflag && i<pce->bufsize)
        {
            do_padding(pce->buf1+i, pce->bufsize-i);
        }
        
        /* memset(pce->buf2, 0, pce->bufsize); */
        /* memset(pce->buf3, 0, pce->bufsize); */
        /* execpengset(ps, buf1, buf2, buf3, eflag); */
        execpengpipe(pce->pp, pce->buf1, pce->buf2, pce->buf3, pce->eflag, multithreading);
        
        k = pce->eflag?(pce->bufsize):i;
        
        if(!pce->eflag && firstblock)
        {
            /* read the header */
            memcpy(&h, pce->buf3, sizeof h);
            off = sizeof h;

            h.headerlen = byte_reorder32(TARGET_BYTEORDER, SYSTEM_BYTEORDER, h.headerlen, 4);
            h.totalsize = byte_reorder64(TARGET_BYTEORDER, SYSTEM_BYTEORDER, h.totalsize, 8);
            h.cksum = byte_reorder64(TARGET_BYTEORDER, SYSTEM_BYTEORDER, h.cksum, 8);
        }
        
        j = write(outh, pce->buf3+off, k-off);
        if(j<0)
        {
            perror(outfn);
            return -1;
        }
        if(k-off!=j)
        {
            fprintf(stderr, "warning: %s: bytes buffered not equal bytes written\n", outfn);
        }
        off = 0;
    }

    if(!pce->eflag)
    {
        pos = lseek(outh, 0, SEEK_CUR);
        lseek(outh, 0, SEEK_SET);
        if(pos>h.totalsize)
        {
            r = ftruncate(outh, h.totalsize);
            if(r)
            {
                perror(outfn);
                return -1;
            }
        }
        lseek(outh, 0, SEEK_SET);
        cksum = wolf64(outh);
        fprintf(stderr, "DEBUG: %08lx (now) %08lx (stored)\n", cksum, h.cksum); 
        if(cksum!=h.cksum)
            return 1;
    }
    
    /* DEBUG_TIMING(1, "peng_cmd_process_1") */
    
    return 0;
}


void peng_cmd_unprep(struct peng_cmd_environment *pce)
{
    destroypengpipe(pce->pp);
    FREE(pce->buf1);
    FREE(pce->buf2);
    FREE(pce->buf3);
}


