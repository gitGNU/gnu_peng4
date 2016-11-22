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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#if LINUX
#include <alloca.h>
#endif
#include <string.h>

#include "sysparm.h"
#include "peng_misc.h"
#include "whirlpool.h"
#include "mt19937ar.h"
#include "peng_ref.h"
#include "libpeng.h"


#define LICENSE "This program comes with ABSOLUTELY NO WARRANTY.\nLicensed under the GNU Public License version 3 or later.\n"


const char *peng_version = "4.01.0078"; /* CHANGEME */


#define MAXFNLEN              1024
#define MIN_LOCRR_SEQ_LEN        8


struct easyset
{
    int  cat;
    char c;
    int  v;
} easysets[] =
{
    {0,'L',0x100}, {0,'M',0x1000}, {0,'H',0x10000}, {0,'X',0x100000}, 
    {1,'L',5}, {1,'M',10}, {1,'H',20}, {1,'X',49}, 
    {2,'L',1}, {2,'M',5}, {2,'H',11}, {2,'X',17},
    
    {-1,0,0}
};


void printversion(void)
{
#if ALPHA || BETA || DEBUG
    printf("DORKINESS=%d SKIP_XOR=%d SKIP_PERMUT=%d USE_MODE_XPX=%d USE_MODE_CBC=%d\n",
           DORKINESS, SKIP_XOR, SKIP_PERMUT, USE_MODE_XPX, USE_MODE_CBC);
    printf("DEBUG=%d\n", DEBUG);
#endif
#if DORKINESS>0 || SKIP_XOR || SKIP_PERMUT || !USE_MODE_XPX || !USE_MODE_CBC
    puts("THIS IS A TESTING VERSION **UNFIT** FOR PRODUCTION USE!");
    puts("*** PARTS OF THE IMPORTANT CODE ARE DISABLED! ***\n");
    puts("If you are not in the inner circle of testers, please do not use this\n"
         "piece of software.");
#elif ALPHA
    puts("This is an ALPHA testing version not meant for production use.\n");
    puts("If you are not in the inner circle of testers, please do not use this\n"
         "piece of software.");
#elif BETA
    puts("This is a BETA testing version not meant for production use.");
#elif DEBUG
    puts("This is a version with DEBUG compiled in.  Do not expect optimum performance.");
#endif
    printf("PENG Version %s, (C)1998-2016 by Klaus-J. Wolf\n", peng_version);
    puts(LICENSE);
}


/* returns array of numbers, first item being the count */
uint32_t *parseints(char *s)
{
    int i0=0,i,j=1,n=1;
    uint32_t *res;
    
    for(i=0; s[i]; i++)
        if(s[i]==',')
            n++;
    res = MALLOC(sizeof(uint32_t)*(n+1));
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


/* returns array of numbers, first item being the count */
uint32_t *parseeasy(char *s, struct easyset es[])
{
    int n=strlen(s), i;
    int cat;
    uint32_t *binparm = malloc((n+1)*sizeof(uint32_t));
    
    binparm[0]=n;
    
    for(cat=0; s[cat]; cat++)
    {
        binparm[cat+1]=0;
        for(i=0; es[i].c; i++)
            if(es[i].cat==cat && es[i].c==s[cat])
            {
                binparm[cat+1]=es[i].v;
                break;
            }
    }
    return binparm;
}


int xmain(int argc, char **argv)
{
    int i,r,h1,h2;
    int opt;
    char *parm = strdup("1024,1,1");
    int eflag = 1;
    int multithreading = 0;
    int delflag;
    char fnmode = 'n';
    uint32_t *binparm;
    uint64_t blksize;
    uint32_t rounds, variations;
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
    while((opt = getopt(argc, argv, "+hVO:drRnvP:mD:")) != -1)
    {
        switch(opt) 
        {
            case 'O':
                parm = strdup(optarg);  /* TODO: leak */
                if(!parm)
                {
                    fputs("out of memory\n", stderr);
                    abort();
                }
                break;
            case 'D':
                debugmask = atoi(optarg);   /* you know what you are doing - ignore errors */
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
            case 'm':
                multithreading = 1;
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
                      "\t\t-O [LMHX][LMHX][LMHX]\n"
                      "\t\t-d\t\tdecrypt\n"
                      "\t\t-r\t\trename input file to infile.bak\n"
                      "\t\t-R\t\treplace input file (dangerous!)\n"
                      "\t\t-n\t\tname output file as infile.{dec|enc} (default)\n"
                      "\t\t-v\t\tincrease verbosity\n"
                      "\t\t-P passphrase\n"
                      "\t\t-m\t\tenable multithreading\n"
                      "\t\t-D mask\t\tset debugging\n", stdout);
                return 1;
        }
    }
    if(optind>=argc)
    {
        fprintf(stderr, "expected argument after options\n");
        return 9;
    }
    
    if(strlen(parm)==3)    /* EASY */
    {
        binparm = parseeasy(parm, easysets);
    }
    else
    {
        binparm = parseints(parm);
    }
    
    r=0;
    for(i=1; i<binparm[0]; i++)
        if(binparm[i]<1)
            r=1;
    
    if(binparm[0]!=3 || r)
    {
        fprintf(stderr, "expected -O parameter as triple numeric separated by commas\nor three letters out of [LMHX]\n");
        return 9;
    }
    blksize = binparm[1];
    rounds = binparm[2];
    variations = binparm[3];
    FREE(binparm);
    
    if(!passphrase)
        passphrase = getpass("PENG Password: ");
    
    if(verbosity>2)
    {
        printf("ID = ");
        for(i=0; i<10; i++)
            printf("%08" PRIx32 " ", mersennetwister_genrand_int32(&mypce.mt));
        puts(""); fflush(stdout);
    }
    
    for(i=optind; i<argc; i++)
    {
        delflag = 0;
        origfn = argv[i];
        if(strlen(origfn)>=MAXFNLEN-5)
        {
            fputs("filename too long\n", stderr);
            return 43;
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
        
        h1 = open(infn, O_RDONLY);
        if(h1<0)
        {
            perror(infn);
            return -1;
        }
        if(access(outfn, R_OK)==0)
        {
            /* outfile exists. in this case, we unlink it first,
               in order to break hardlinks */
            r = unlink(outfn);
            if(r<0)
            {
                perror(outfn);
                return -1;
            }
        }
        h2 = open(outfn, O_RDWR|O_CREAT|O_EXCL, 0600);
        if(h2<0)
        {
            perror(outfn);
            close(h1);
            return -1;
        }
        
#if USE_UNENCRYPTED_HEADER
        /* mypce.off = sizeof(mypce.htrx0); */
        if(eflag)
        {
            mypce.htrx0.blksize = blksize;
            mypce.htrx0.rounds = rounds;
            mypce.htrx0.variations = variations;
            mypce.htrx0.extra = 0;
            r = peng_preliminary_header_write_convenience(&mypce, h2);
        }
        else
        {
            r = peng_preliminary_header_read_convenience(&mypce, h1);
            blksize = mypce.htrx0.blksize;
            rounds = mypce.htrx0.rounds;
            variations = mypce.htrx0.variations;
            /* total -= mypce.off; */
        }
        if(r!=0)
        {
            switch(r)
            {
                case ERROR_SYSTEM_INFILE:
                    perror(infn);
                    break;
                case ERROR_SYSTEM_OUTFILE:
                    perror(outfn);
                    break;
                case ERROR_MAGIC:
                    fprintf(stderr, "%s: bad magic\n", infn);
                    break;
                default:
                    fprintf(stderr, "unspecified or undefined error %d\n", r);
            }
            return 10;
        }
        /* printf("DEBUG: infile pos = %lu,%lu\n", (unsigned long)lseek(h1, 0, SEEK_CUR), (unsigned long)lseek(h2, 0, SEEK_CUR)); */
#else
        /* mypce.off = 0; */
#endif
        peng_cmd_prep(&mypce, blksize, rounds, variations, passphrase, eflag);
    
        if(verbosity>0)
        {
            printf("blksize=%" PRIu64 " variations=%" PRIu32 " rounds=%" PRIu32 "\n", blksize, variations, rounds);
        }
        

        if(verbosity>0)
        {
            printf("%30s   -%c->    %-30s\n", infn, eflag ? 'e':'d', outfn);
            fflush(stdout);
        }
        r = peng_cmd_process(&mypce, infn, h1, outfn, h2, multithreading, MIN_LOCRR_SEQ_LEN);
        
        close(h1);
        close(h2);
        
        if(r!=0)  /* error */
        {
            switch(r)
            {
                case ERROR_SYSTEM_INFILE:
                    perror(infn);
                    break;
                case ERROR_SYSTEM_OUTFILE:
                    perror(outfn);
                    break;
                case ERROR_CHECKSUM:
                    fprintf(stderr, "%s: checksum mismatch\n", argv[i]);
                    break;
                case ERROR_MAGIC:
                    fprintf(stderr, "%s: magic missing (password wrong?)\n", argv[i]);
                    break;
                case ERROR_COMPAT:
                    fprintf(stderr, "%s: compatibility (version or capabilities) failure\n", argv[i]);
                    break;
                default:
                    fprintf(stderr, "unspecified or undefined error %d\n", r);
            }
            return 10;
        }
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


/* this translates return values from xmain() (which can be negative
 * which translates into confusion on shell level)
 */
int main(int argc, char **argv)
{
    int r = xmain(argc, argv);
    if(r<0) return 17;
    return r;
}
