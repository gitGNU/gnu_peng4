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
#include "libpeng.h"
#include "wolf64.h"


/* global */ int verbosity = 0;
/* global */ int debugmask = 0;

static int init_done = 0;


#define COMBINED_DIGEST_SIZE (WHIRLPOOL_DIGESTBYTES+SHA512_DIGEST_SIZE)
#define PENG_MAGIC UINT32_C(0xec1f4a11)
#define PENG_VER UINT16_C(0)
#define PENG_CAP UINT16_C(0)


void peng_unit_prep(void)
{
    if(init_done)
        return;
    init_done=1;
    /* ... */
}


int peng_preliminary_header_read_convenience(struct peng_cmd_environment *pce, int f)
{
    struct peng_file_header_unencrypted h;
    int r;
    
    peng_unit_prep();
    
    r=read(f, &h, sizeof h);
    if(r<0)
        return -1;
    pce->htrx0.blksize = cvt_to_system64(h.blksize);
    pce->htrx0.rounds = cvt_to_system64(h.rounds);
    pce->htrx0.variations = cvt_to_system64(h.variations);
    pce->htrx0.extra = cvt_to_system64(h.extra);
    return 0;
}


int peng_preliminary_header_write_convenience(struct peng_cmd_environment *pce, int f)
{
    struct peng_file_header_unencrypted h;
    int r;
    
    peng_unit_prep();
    
    h.blksize = cvt_from_system64(pce->htrx0.blksize);
    h.rounds = cvt_from_system64(pce->htrx0.rounds);
    h.variations = cvt_from_system64(pce->htrx0.variations);
    h.extra = cvt_from_system64(pce->htrx0.extra);
    r=write(f, &h, sizeof h);
    if(r<0)
        return -2;
    return 0;
}


/* attention ! passphrase will be erased ! */
void peng_cmd_prep(struct peng_cmd_environment *pce, uint64_t blksize, uint32_t rounds, uint32_t variations, char *passphrase, int eflag)
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
    
    rectify32(SYSTEM_BYTEORDER32, TARGET_BYTEORDER32, combined, COMBINED_DIGEST_SIZE);
    
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
    int i,j,k,r;
    int firstblock;
    uint32_t num=0, off=0;
    uint64_t pos=0, cksum;
    struct peng_file_header h;
    struct peng_file_header hwrit;
    
    memset(&h, 0, sizeof h);  /* valgrind owed */
    memset(&hwrit, 0, sizeof hwrit);
    /* DEBUG_TIMING(1, "peng_cmd_process_0") */
    
    if(pce->eflag)
    {
        lseek(inh, 0, SEEK_SET);
        h.cksum = wolf64(inh);
        h.totalsize = lseek(inh, 0, SEEK_END);   /* it is already at the end... */
        h.headerlen = sizeof h; 
        h.magic = PENG_MAGIC;
        h.cap = PENG_CAP;
        h.ver = PENG_VER;
        
        memcpy(&pce->htrx, &h, sizeof(pce->htrx));
        lseek(inh, 0, SEEK_SET);
        
        hwrit.headerlen = cvt_from_system64(h.headerlen);
        hwrit.totalsize = cvt_from_system64(h.totalsize);
        hwrit.cksum = cvt_from_system64(h.cksum);
        hwrit.magic = cvt_from_system32(h.magic);
        hwrit.extra = cvt_from_system32(h.extra);
        hwrit.cap = cvt_from_system16(h.cap);
        hwrit.ver = cvt_from_system16(h.ver);
        
        memcpy(pce->buf1, &hwrit, sizeof hwrit);
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
            /* perror(infn); */
            return ERROR_SYSTEM_INFILE;
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

            h.ver = cvt_to_system16(h.ver);
            h.cap = cvt_to_system16(h.cap);
            h.magic = cvt_to_system32(h.magic);
            h.extra = cvt_to_system32(h.extra);
            h.headerlen = cvt_to_system32(h.headerlen);
            h.totalsize = cvt_to_system64(h.totalsize);
            h.cksum = cvt_to_system64(h.cksum);
            
            memcpy(&pce->htrx, &h, sizeof(pce->htrx));
            
            if(h.magic!=PENG_MAGIC)
            {
                /* fprintf(stderr, "DEBUG: FAILED: ver=%"PRIx16" cap=%"PRIx16" magic=%"PRIx32"\n", h.ver, h.cap, h.magic); */
                return ERROR_MAGIC;
            }
            if(h.ver>PENG_VER || h.cap!=PENG_CAP)
            {
                /* fprintf(stderr, "DEBUG: FAILED: ver=%"PRIx16" cap=%"PRIx16" magic=%"PRIx32"\n", h.ver, h.cap, h.magic);  */
                return ERROR_COMPAT;
            }
        }
        
        j = write(outh, pce->buf3+off, k-off);
        if(j<0)
        {
            /* perror(outfn); */
            return ERROR_SYSTEM_OUTFILE;
        }
        if(k-off!=j)
        {
            fprintf(stderr, "warning: %s: bytes buffered not equal bytes written\n", outfn);
        }
        off = 0;
    }

    if(!pce->eflag)
    {
        /* fprintf(stderr, "DEBUG: h.totalsize = %"PRIu64"\n", h.totalsize); */
        pos = lseek(outh, 0, SEEK_CUR);
        lseek(outh, 0, SEEK_SET);
        if(pos>h.totalsize)
        {
            /* fprintf(stderr, "DEBUG: ftruncate() to %"PRIu64"\n", h.totalsize); */
            r = ftruncate(outh, h.totalsize);
            if(r)
            {
                /* perror(outfn); */
                return ERROR_SYSTEM_OUTFILE;
            }
        }
        lseek(outh, 0, SEEK_SET); /* some ftruncate()s work by re-positioning */
        cksum = wolf64(outh);
        /* fprintf(stderr, "DEBUG: %08lx (now) %"PRIx64" (stored)\n", cksum, h.cksum); */
        if(cksum!=h.cksum)
            return ERROR_MAGIC;
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


