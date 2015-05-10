#define MERSENNETWISTER_N 624
#define MERSENNETWISTER_M 397


struct mersennetwister
{
    unsigned long mt[MERSENNETWISTER_N]; /* the array for the state vector  */
    int mti;    /* =MERSENNETWISTER_N+1; * mti==N+1 means mt[N] is not initialized */
};


void mersennetwister_init_genrand(struct mersennetwister *mt, unsigned long s);
void mersennetwister_init_by_array(struct mersennetwister *mt, unsigned long init_key[], int key_length);
unsigned long mersennetwister_genrand_int32(struct mersennetwister *mt);
long mersennetwister_genrand_int31(struct mersennetwister *mt);
double mersennetwister_genrand_real1(struct mersennetwister *mt);
double mersennetwister_genrand_real2(struct mersennetwister *mt);
double mersennetwister_genrand_real3(struct mersennetwister *mt);
double mersennetwister_genrand_res53(struct mersennetwister *mt);
unsigned long long mersennetwister_genrand_int64(struct mersennetwister *mt);
