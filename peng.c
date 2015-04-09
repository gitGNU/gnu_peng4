
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


int main(int argc, char **argv)
{
    int h1, h2, i, j, num=0, eflag=0;
    char *buf1 = MALLOC(BUFSIZE);
    char *buf2 = MALLOC(BUFSIZE);
    char *buf3 = MALLOC(BUFSIZE);
    struct pengset *ps;
    struct mersennetwister mt;
    
    if(argc<5)
    {
        fprintf(stderr, "usage: %s infile outfile pass e|d\n", argv[0]);
        return 1;
    }
    
    i = strlen(argv[3]);
    memset(buf1, 0, BUFSIZE);
    strncpy(buf1, argv[3], j=(i>BUFSIZE)?(BUFSIZE):i);
    mersennetwister_init_by_array(&mt, (unsigned long *)buf1, (j+3)/4);   /* TODO: fix byte order */
    memset(buf1, 0, BUFSIZE);

    eflag = !strcmp(argv[4], "e");
    
    printf("%s -%c-> %s\n", argv[1], eflag?'e':'d', argv[2]);
    
    ps = genpengset(BUFSIZE, &mt);
    
    h1 = open(argv[1], O_RDONLY);
    if(h1<0)
    {
        perror(argv[1]);
        return 99;
    }
    h2 = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if(h2<0)
    {
        perror(argv[2]);
        close(h1);
        return 99;
    }
    for(;;)
    {
        printf("block #%d\r", ++num);
        fflush(stdout);
        
        memset(buf1, 0, BUFSIZE);
        i = read(h1, buf1, BUFSIZE);
        if(i<0)
        {
            perror(argv[1]);
            return 90;
        }
        if(i<=0)
            break;
        
        if(!eflag && i<BUFSIZE)
        {
            fputs("warning: expected a full block while reading for decryption\n", stderr);
        }
        
        memset(buf2, 0, BUFSIZE);
        memset(buf3, 0, BUFSIZE);
        execpengset(ps, buf1, buf2, buf3, eflag);
        
        j = write(h2, buf3, eflag?(BUFSIZE):i);
        if(j<0)
        {
            perror(argv[2]);
            return 90;
        }
        if(i!=j)
        {
            fputs("warning: bytes read != bytes written\n", stderr);
        }
    }
    close(h1);
    close(h2);
    destroypengset(ps);
    free(buf1);
    free(buf2);
    free(buf3);

    return 0;
}
