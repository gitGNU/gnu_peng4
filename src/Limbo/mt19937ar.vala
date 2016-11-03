
class MersenneTwister : GLib.Object
{
    public const uint N=624;
    public const uint M=397;

    protected uint32[] mt = new uint32[N]; /* the array for the state vector  */
    protected uint32 mti;    /* =N+1; * mti==N+1 means mt[N] is not initialized */

    private const uint32 MATRIX_A = (uint32)0x9908b0dful;   /* constant vector a */
    private const uint32 UPPER_MASK = (uint32)0x80000000ul; /* most significant w-r bits */
    private const uint32 LOWER_MASK = (uint32)0x7ffffffful; /* least significant r bits */
    
    
    public void init_genrand(uint32 s)
    {
        mti=N+1;
        mt[0]= s & 0xffffffff;
        for (mti=1; mti<N; mti++)
        {
            mt[mti] = ((uint32)1812433253ul * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
            /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
            /* In the previous versions, MSBs of the seed affect   */
            /* only MSBs of the array mt[].                        */
            /* 2002/01/09 modified by Makoto Matsumoto             */
            mt[mti] &= (uint32)0xfffffffful;
            /* for >32 bit machines */
        }
    }
    
    public void init_by_array(uint32[] init_key, int key_length)
    {
        int i, j, k;
        init_genrand(19650218);
        i=1; j=0;
        k = ((int)N>key_length ? (int)N : key_length);
        for (; k!=0; k--) {
            mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * (uint32)1664525)) + init_key[j] + j; /* non linear */
            mt[i] &= (uint32)0xfffffffful; /* for WORDSIZE > 32 machines */
            i++; j++;
            if (i>=N) { mt[0] = mt[N-1]; i=1; }
            if (j>=key_length) j=0;
        }
        for (k=(int)N-1; k!=0; k--) {
            mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * (uint32)1566083941ul)) - i; /* non linear */
            mt[i] &= (uint32)0xfffffffful; /* for WORDSIZE > 32 machines */
            i++;
            if (i>=N) { mt[0] = mt[N-1]; i=1; }
        }

        mt[0] = (uint32)0x80000000ul; /* MSB is 1; assuring non-zero initial array */ 
    }

    private uint32[] mag01 = {0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */
    
    public uint32 genrand_int32()
    {
        uint32 y;

        if (mti >= N) { /* generate N words at one time */
            int kk;

            if (mti == N+1)   /* if init_genrand() has not been called, */
                init_genrand((uint32)5489UL); /* a default initial seed is used */

            for (kk=0;kk<N-M;kk++) {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
            }
            for (;kk<N-1;kk++) {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
            }
            y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
            mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

            mti = 0;
        }
    
        y = mt[mti++];

        /* Tempering */
        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680;
        y ^= (y << 15) & 0xefc60000;
        y ^= (y >> 18);

        return y;
    }
    
    public uint32 genrand_int32_strong(uint32 mx)
    {
        uint32 u=0;
        uint32 mask = (uint32)0xfffffffful;
        
        /* speed it up a little bit */
        if(mx<=0x1000000)
        {
            if(mx<=0x10000)
            {
                if(mx<=0x100)
                    mask = 0xff;
                else
                    mask = 0xffff;
            }
            else
                mask = 0xffffff;
        }
        do
            u = genrand_int32() & mask;
        while(u>=mx);
        return u;
    }

    public long genrand_int31()
    {
        return (long)(genrand_int32()>>1);
    }

    public double genrand_real1()
    {
        return genrand_int32()*(1.0/4294967295.0); 
        /* divided by 2^32-1 */ 
    }
    
    public double genrand_real2()
    {
        return genrand_int32()*(1.0/4294967296.0); 
        /* divided by 2^32 */
    }

    public double genrand_real3()
    {
        return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0); 
        /* divided by 2^32 */
    }

    public double genrand_res53()
    { 
        uint32 a=genrand_int32()>>5, b=genrand_int32()>>6; 
        return (a*67108864.0+b)*(1.0/9007199254740992.0); 
    } 
    
    public uint64 genrand_int64()
    {
        uint64 res = genrand_int32();
        
        res<<=32;
        res|=genrand_int32();
        return res;
    }

}


