/*
  countbits.c
*/

#include <stdio.h>


unsigned bytebits(int c)
{
    unsigned n=0;
    
    if(c&1) n++;
    if(c&2) n++;
    if(c&4) n++;
    if(c&8) n++;
    if(c&0x10) n++;
    if(c&0x20) n++;
    if(c&0x40) n++;
    if(c&0x80) n++;
    return n;
}


int main(int argc, const char *argv[])
{
    unsigned long long num;
    int i, c;
    FILE *f;

    for(i=1; i<argc; i++)
    {
        num = 0;
        f=fopen(argv[i], "r");
        if(f==NULL)
        {
            perror(argv[i]);
            continue;
        }
        for(;;)
        {
            c = fgetc(f);
            if(c==EOF)
                break;
            num += bytebits(c);
        }
        fclose(f);
        printf("%s: %llu\n", argv[i], num);
    }
    return 0;
}
