/*
 * whirlpool.hh
 */


#define WHIRLPOOL_DIGESTBYTES 64
#define WHIRLPOOL_DIGESTBITS  (8*WHIRLPOOL_DIGESTBYTES) /* 512 */

#define WHIRLPOOL_WBLOCKBYTES 64
#define WHIRLPOOL_WBLOCKBITS  (8*WHIRLPOOL_WBLOCKBYTES) /* 512 */

#define WHIRLPOOL_LENGTHBYTES 32
#define WHIRLPOOL_LENGTHBITS  (8*WHIRLPOOL_LENGTHBYTES) /* 256 */


struct whirlpool
{
/* private: */
    uint8_t  bitLength[WHIRLPOOL_LENGTHBYTES]; /* global number of hashed bits (256-bit counter) */
    uint8_t  buffer[WHIRLPOOL_WBLOCKBYTES];    /* buffer of data to hash */
    int      bufferBits;             /* current number of bits on the buffer */
    int      bufferPos;              /* current (possibly incomplete) byte slot on the buffer */
    uint64_t hash[WHIRLPOOL_DIGESTBYTES/8];    /* the hashing state */
};


void whirlpool_processbuffer(struct whirlpool *wp);
void whirlpool_init(struct whirlpool *wp);
void whirlpool_add(struct whirlpool *wp, const unsigned char * const source, unsigned long sourceBits);
void whirlpool_finalize(struct whirlpool *wp, unsigned char * const result);
