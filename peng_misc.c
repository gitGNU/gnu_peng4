#include <stdlib.h>

#include "peng_glob.h"


void *chkmalloc(unsigned x)
{
    void *p = malloc(x);
    if(!p)
    {
        fputs("out of memory error\n", stderr);
        abort();
    }
    return p;
}

