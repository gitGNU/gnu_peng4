
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
#include "peng_ref.h"


const char *peng_version = "4.01.00.009"; /* CHANGEME */


#define MAXFNLEN 1024


/* global */ int verbosity = 0;


struct peng_cmd_environment
{
    struct pengpipe *pp;
    struct mersennetwister mt;
    unsigned char *buf1, *buf2, *buf3;
    unsigned blksize, bufsize;
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

    if(verbosity>2)
    {
        printf("passphrase = %s\n", passphrase);
        printf("whirlpool = %s\n", whirlpool_hexhash(&wp));
        fflush(stdout);
    }
    
    mersennetwister_init_by_array(&pce->mt, (unsigned long *)digest, WHIRLPOOL_DIGESTBYTES/sizeof(unsigned long));  /* TODO byte order, packing */
    
    pce->pp = genpengpipe(blksize, rounds, variations, &pce->mt);
    pce->blksize = blksize;    /* again, this is the third time this value is stored (in variants) */
    pce->bufsize = getbufsize(pce->pp);
    pce->eflag = eflag?1:0;
    
    pce->buf1 = MALLOC(pce->bufsize);
    pce->buf2 = MALLOC(pce->bufsize);
    pce->buf3 = MALLOC(pce->bufsize);
    
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
    if(verbosity>0)
    {
        printf("%30s   -%c->    %-30s\n", infn, pce->eflag ? 'e':'d', outfn);
        fflush(stdout);
    }
    for(;;)
    {
        if(verbosity>1)
        {
            printf("block #%u\r", ++num);
            fflush(stdout);
        }
        
        memset(pce->buf1, 0, pce->bufsize);
        i = read(h1, pce->buf1, pce->bufsize);
        if(i<0)
        {
            perror(infn);
            return -1;
        }
        if(i<=0)
            break;
        
        if(i<pce->bufsize)
            do_padding(pce->buf1+i, pce->bufsize-i);
        
        if(!pce->eflag && i<pce->bufsize)
        {
            fputs("warning: expected a full block while reading for decryption\n", stderr);
        }
        
        memset(pce->buf2, 0, pce->bufsize);
        memset(pce->buf3, 0, pce->bufsize);
        /* execpengset(ps, buf1, buf2, buf3, eflag); */
        execpengpipe(pce->pp, pce->buf1, pce->buf2, pce->buf3, pce->eflag);
        
        j = write(h2, pce->buf3, pce->eflag?(pce->bufsize):i);    /* TODO: this looks like a problem */
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
    destroypengpipe(pce->pp);
    FREE(pce->buf1);
    FREE(pce->buf2);
    FREE(pce->buf3);
}


void printversion(void)
{
#if DORKY || SKIP_XOR || SKIP_PERMUT
    puts("THIS IS A TESTING VERSION //UNFIT\\\\ FOR PRODUCTION USE!");
    puts("If you are not in the inner circle of testers, please do not use this\n"
         "piece of software.");
#elif ALPHA
    puts("This is a ALPHA testing version not meant for production use.");
    puts("If you are not in the inner circle of testers, please do not use this\n"
         "piece of software.");
#elif BETA
    puts("This is a BETA testing version not meant for production use.");
#elif DEBUG
    puts("This is a version with DEBUG compiled in.  Do not expect optimum performance.");
#endif
    printf("PENG Version %s, (C)1998-2015 by Klaus-J. Wolf\n", peng_version);
}


/* returns array of numbers, first being the count */
unsigned *parseints(char *s)
{
    int i0=0,i,j=1,n=1;
    unsigned *res;
    
    for(i=0; s[i]; i++)
        if(s[i]==',')
            n++;
    res = MALLOC(sizeof(unsigned)*(n+1));
    res[0] = n;
    for(i=0; s[i]; i++)
    {
        if(s[i]==',')
        {
            s[i]=0;
            res[j++]=atoi(s+i0);
            s[i]=',';
            i0=i+1;
        }
    }
    res[j]=atoi(s+i0);
    return res;
}


int main(int argc, char **argv)
{
    int i,r;
    int opt;
    char *parm = "1024,1,1";
    int eflag = 1;
    int delflag;
    char fnmode = 'n';
    unsigned *binparm;
    unsigned blksize, rounds, variations;
    struct peng_cmd_environment mypce;
    char *origfn, infn[MAXFNLEN], outfn[MAXFNLEN], *passphrase=NULL;

    if(argc<=1)
    {
        fprintf(stderr, "use option -h for help\n");
        return 9;
    }
    /* GNU: */
    /* + means: parse POSIXLY_CORRECT, stopping with the first non-option */
    /* - means: give opt==1 for any non-option parameter */
    while((opt = getopt(argc, argv, "+hVO:drRnvP:")) != -1)
    {
        switch(opt) 
        {
            case 'O':
                parm = strdup(optarg);  /* TODO leak */
                if(!parm)
                {
                    fputs("out of memory\n", stderr);
                    abort();
                }
                break;
            case 'r':
            case 'R':
            case 'n':
                fnmode = opt;
                break;
            case 'd':
                eflag = 0;
                break;
            case 'V':
                printversion();
                return 0;
            case 'v':
                verbosity++;
                break;
            case 'P':
                passphrase = strdup(optarg);  /* TODO leak */
                if(!passphrase)
                {
                    fputs("out of memory\n", stderr);
                    abort();
                }
                break;
            case 1:
                abort();   /* there shouldn't be unhandled parameters here */
            case 'h':
                /* no break */
            default:  /* '?' */
                fprintf(stderr, "usage: %s [option] infile...\n", argv[0]);
                fputs("options:\t-h\t\tthis help\n"
                      "\t\t-V\t\tdisplay version information\n"
                      "\t\t-O blocksize,rounds,variations\n"
                      "\t\t-d\t\tdecrypt\n"
                      "\t\t-r\t\trename input file to infile.bak\n"
                      "\t\t-R\t\treplace input file (dangerous!)\n"
                      "\t\t-n\t\tname output file as infile.{dec|enc} (default)\n"
                      "\t\t-v\t\tincrease verbosity\n"
                      "\t\t-P passphrase\n", stdout);
                return 1;
        }
    }
    if(optind>=argc)
    {
        fprintf(stderr, "expected argument after options\n");
        return 9;
    }
    
    binparm = parseints(parm);
    
    if(binparm[0]!=3)
    {
        fprintf(stderr, "expected -O parameter options as triple numeric separated by commas\n");
        return 9;
    }
    blksize = binparm[1];
    rounds = binparm[2];
    variations = binparm[3];
    FREE(binparm);
    
    if(!passphrase)
        passphrase = getpass("PENG Password: ");
    
    peng_cmd_prep(&mypce, blksize, rounds, variations, passphrase, eflag);
    
    for(i=optind; i<argc; i++)
    {
        delflag = 0;
        origfn = argv[i];
        if(strlen(origfn)>=MAXFNLEN-5)
        {
            fputs("filename too long\n", stderr);
            return 100;
        }
        
        switch(fnmode)
        {
            case 'R':
                /* Unix only: delete the input file, create a new file with the same name */
                delflag = 1;
                /* no break */
            case 'r':
                /* first, rename input file to .bak, then create a new file with the original name */
                strcpy(outfn, origfn);
                strcpy(infn, origfn);
                strcat(infn, ".bak");
                r = rename(outfn, infn);
                if(r<0)
                {
                    perror(outfn);
                    return 99;
                }
                break;
            case 'n':
                /* output file is .enc or .dec depending on eflag */
                strcpy(infn, origfn);
                strcpy(outfn, origfn);
                strcat(outfn, eflag?".enc":".dec");
                break;
            default:
                abort();
        }
        r = peng_cmd_process(&mypce, infn, outfn);
        if(r<0)
        {
            /* perror(argv[i]); <<< this is done in the process */
            return 19;
        }
        if(delflag)
            remove(infn);
    }

    peng_cmd_unprep(&mypce);
    
    return 0;
}
