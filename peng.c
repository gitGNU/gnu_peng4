
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>
#include <string.h>

#include "peng_glob.h"
#include "whirlpool.h"
#include "mt19937ar.h"
#include "slowpeng.h"


struct peng_cmd_environment
{
    struct pengpipe *pp;
    struct mersennetwister mt;
    unsigned char *buf1, *buf2, *buf3;
    unsigned blksize;
    int eflag;
};


/* attention ! passphrase will be erased ! */
void peng_cmd_prep(struct peng_cmd_environment *pce, unsigned blksize, unsigned rounds, unsigned variations, char *passphrase, int eflag)
{
    struct whirlpool wp;
    unsigned char digest[WHIRLPOOL_DIGESTBYTES];
    
    whirlpool_init(&wp);
    whirlpool_add(&wp, (unsigned char *) passphrase, strlen(passphrase)*8);
    whirlpool_finalize(&wp, digest);

    mersennetwister_init_by_array(&pce->mt, (unsigned long *)digest, WHIRLPOOL_DIGESTBYTES/4);  /* TODO byte order, packing */
    
    pce->pp = genpengpipe(blksize, rounds, variations, &pce->mt);
    pce->blksize = blksize;    /* again, this is the third time this value is stored */
    pce->eflag = eflag?1:0;
    
    pce->buf1 = MALLOC(blksize);
    pce->buf2 = MALLOC(blksize);
    pce->buf3 = MALLOC(blksize);
    
    memset(passphrase, 0, strlen(passphrase));
    memset(&wp, 0, sizeof wp);
    memset(digest, 0, WHIRLPOOL_DIGESTBYTES);
}


int peng_cmd_process(struct peng_cmd_environment *pce, const char *infn, const char *outfn)
{
    int h1, h2;
    int i,j;
    unsigned num=0;
    
    h1 = open(infn, O_RDONLY);
    if(h1<0)
    {
        perror(infn);
        return -1;
    }
    h2 = open(outfn, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if(h2<0)
    {
        perror(outfn);
        close(h1);
        return -1;
    }
    printf("%30s   -%c->    %-30s\n", infn, pce->eflag ? 'e':'d', outfn);
    for(;;)
    {
        printf("block #%u\r", ++num);
        fflush(stdout);
        
        memset(pce->buf1, 0, pce->blksize);
        i = read(h1, pce->buf1, pce->blksize);
        if(i<0)
        {
            perror(infn);
            return -1;
        }
        if(i<=0)
            break;
        
        if(!pce->eflag && i<pce->blksize)
        {
            fputs("warning: expected a full block while reading for decryption\n", stderr);
        }
        
        memset(pce->buf2, 0, pce->blksize);
        memset(pce->buf3, 0, pce->blksize);
        /* execpengset(ps, buf1, buf2, buf3, eflag); */
        execpengpipe(&pce->pp, pce->buf1, pce->buf2, pce->buf3, pce->eflag);
        
        j = write(h2, pce->buf3, pce->eflag?(pce->blksize):i);
        if(j<0)
        {
            perror(outfn);
            return -1;
        }
        if(i!=j)
        {
            fputs("warning: bytes read != bytes written\n", stderr);
        }
    }
    close(h1);
    close(h2);
    return 0;
}


void peng_cmd_unprep(struct peng_cmd_environment *pce)
{
    destroypengset(pce->pp);
    FREE(pce->buf1);
    FREE(pce->buf2);
    FREE(pce->buf3);
}


int main(int argc, char **argv)
{
    return 0;
}
