#define MERSENNETWISTER_N 624
#define MERSENNETWISTER_M 397


struct mersennetwister
{
    uint32_t mt[MERSENNETWISTER_N]; /* the array for the state vector  */
    int mti;    /* =MERSENNETWISTER_N+1; * mti==N+1 means mt[N] is not initialized */
};


void mersennetwister_init_genrand(struct mersennetwister *mt, uint32_t s);
void mersennetwister_init_by_array(struct mersennetwister *mt, uint32_t init_key[], int key_length);
uint32_t mersennetwister_genrand_int32(struct mersennetwister *mt);
uint32_t mersennetwister_genrand_int32_strong(struct mersennetwister *mt, uint32_t mx);
long mersennetwister_genrand_int31(struct mersennetwister *mt);
double mersennetwister_genrand_real1(struct mersennetwister *mt);
double mersennetwister_genrand_real2(struct mersennetwister *mt);
double mersennetwister_genrand_real3(struct mersennetwister *mt);
double mersennetwister_genrand_res53(struct mersennetwister *mt);
uint64_t mersennetwister_genrand_int64(struct mersennetwister *mt);
