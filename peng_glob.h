#define MALLOC(x) chkmalloc(x)
#define MALLOCA(x) alloca(x)
#define FREE(x) free(x)
#define FREEA(x)


extern int verbosity;


void *chkmalloc(unsigned x);

void memxor(void *dst0, const void *src0, unsigned sz0);

void do_padding(void *buf0, unsigned sz0);
