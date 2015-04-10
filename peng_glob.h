#define MALLOC(x) chkmalloc(x)
#define MALLOCA(x) alloca(x)
#define FREE(x) free(x)
#define FREEA(x)


extern int verbosity;


void *chkmalloc(unsigned x);
