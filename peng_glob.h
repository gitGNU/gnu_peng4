#define MALLOC(x) chkmalloc(x)
#define MALLOCA(x) alloca(x)
#define FREE(x) free(x)
#define FREEA(x)


void *chkmalloc(unsigned x);
