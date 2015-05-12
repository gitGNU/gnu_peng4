#define MALLOC(x) chkmalloc(x)
#define MALLOCA(x) alloca(x)
#define FREE(x) free(x)
#define FREEA(x)


extern int verbosity;


void *chkmalloc(unsigned x);

int mymemcmp(const void *abuf0, const void *bbuf0, unsigned sz0);

void memxor(void *dst0, const void *src0, unsigned sz0);

unsigned do_padding(void *buf0, unsigned sz0, const unsigned long *marker, unsigned nmarker, unsigned marker_byteoffset);

int locrr(void *buf, unsigned sz, const unsigned long *marker, unsigned nmarker, int minmatch);
