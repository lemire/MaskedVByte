#include <stdint.h>
#include <x86intrin.h>
typedef __m256i vec256i_t;
typedef __m128i vec128i_t;

#ifdef IACA 
#include </opt/intel/iaca/include/iacaMarks.h>
#else // not IACA
#undef IACA_START
#define IACA_START
#undef IACA_END
#define IACA_END
#endif // not IACA

typedef struct {
    uint8_t consumed;  // only low 6-bits affect shift (high 2-bits available)
    uint8_t index;
} vbyte_lookup_t;
vbyte_lookup_t vbyte_lookup_table[4096]; // 2^12 * 2B
  
// Could be shrunk to 2 x uint32_t or smaller if cache space is at a
// premium, but keeping them full size helps minimize pressure on Port 5.
typedef struct {
    vec128i_t low;
    vec128i_t high;
} vbyte_shuffle_t;
vbyte_shuffle_t vbyte_shuffle_table[256]; // 2^8
uint8_t vbyte_valid_table[256];

#define LOOKUP_MASK_BITS (0xCF)  // low 12 bits

// shuffle VByte compressed data to correct order (still has high-bit gaps)
static inline vec256i_t _shuffleRaw(vec128i_t data, uint8_t index) {
    vec128i_t *shuffleLowPtr = &(vbyte_shuffle_table[index].low);
    vec128i_t *shuffleHighPtr = &(vbyte_shuffle_table[index].high);

    // shuffle bytes from dataPtr to create up to 8 32-bit ints
    vec128i_t dataLow = _mm_shuffle_epi8(data, *shuffleLowPtr);
    vec128i_t dataHigh = _mm_shuffle_epi8(data, *shuffleHighPtr);

    // combine 128-bit dataLow and dataHigh into single 256-bit vector
    return _mm256_inserti128_si256(_mm256_castsi128_si256(dataLow), dataHigh, 1);
}

// shrink each element to remove gaps at bits 7, 14, 21, and 28
static inline vec256i_t _shrinkGaps(vec256i_t data) {
    const vec256i_t mask0 = _mm256_set1_epi32(0x7F << 0);  // keep bits 0-6
    const vec256i_t mask1 = _mm256_set1_epi32(0x7F << 7);  // keep bits 7-13
    const vec256i_t mask2 = _mm256_set1_epi32(0x7F << 14); // keep bits 14-20
    const vec256i_t mask3 = _mm256_set1_epi32(0x7F << 21); // keep bits 21-28

    // shift and mask bytes for individual bytes in each element
    vec256i_t data0 = data;                        // element bits 0-7
    vec256i_t data1 = _mm256_srli_epi32(data0, 1); // element bits 8-15
    vec256i_t data2 = _mm256_srli_epi32(data0, 2); // element bits 16-23
    vec256i_t data3 = _mm256_srli_epi32(data0, 3); // element bits 24-31

    // NOTE: issue all shifts before any masks for better port utilization

    // mask pre-shifted data to leave 7-bit chunks than can be combined
    data0 = _mm256_and_si256(data0, mask0);
    data1 = _mm256_and_si256(data1, mask1);
    data2 = _mm256_and_si256(data2, mask2);
    data3 = _mm256_and_si256(data3, mask3);

    // combine "compressed" data (high bits removed)
    data0 = _mm256_or_si256(data0, data1);
    data2 = _mm256_or_si256(data2, data3);
    data = _mm256_or_si256(data0, data2); 

    return data; // vector containing 0-8 valid 32-bit integers
}

// delta decode vec [A B C D : E F G H] using prev [P P P P : P P P P]
static inline vec256i_t _inverseDelta(vec256i_t vec, vec256i_t prev) {
    //  vec256i_t uint32 ordering :  {0x0000000100000000, 0x000000300000002,
    //                                0x0000000500000004, 0x000000700000006}
    const vec256i_t maskKeepThird =  {0x0000000000000000, 0xFFFFFFFF00000000,
                                      0x0000000000000000, 0x0000000000000000};
    const vec256i_t broadcastThird = {0x0000000000000000, 0x0000000000000000,
                                      0x0000000300000003, 0x0000000300000003};

    vec256i_t add = _mm256_slli_si256(vec, 4);  // [- A B C : - E F G]
    vec = _mm256_add_epi32(vec, add);           // [A AB BC CD : E EF FG GH]
    add = _mm256_slli_si256(vec, 8);            // [- - A AB : - - E EF]
    vec = _mm256_add_epi32(vec, add);           // [A AB ABC ABCD : E EF EFG EFGH]
    add = _mm256_and_si256(vec, maskKeepThird); // [- - - ABCD : - - - -]
    add = _mm256_permutevar8x32_epi32(add, broadcastThird); // [- - - - : ABCD ABCD ABCD ABCD]
    vec = _mm256_add_epi32(vec, prev); // [PA PAB PABC PABCD : PE PEF PEFG PEFGH]
    vec = _mm256_add_epi32(vec, add);  // [PA PAB PABC PABCD : PABCDE PABCDEF PABCDEFG PABCDEFGH]

    return vec;
}

// try loading up to 8 total 32-bit vectors (stopping if insufficient safe space remains)
// from each pair of vectors, create a 64-bit number from the high bits of each byte
// WARNING: caller is expected to check if safeInputBytesRemaining > 32
//          this function expects that it is safe to read at least 32B
// returns number of bytes read. caller should advance bytePtr reduce safeBytes by this.
// always returns 256 except for final call when fewer than 32 safeBytes will remain
// if (remainingBits = _realoadBuffer()) < 256 { do final iteration }
// FUTURE: better to stick with aligned reads and shift (ARM portability?)
static inline uint64_t _reloadBuffer (uint8_t *bytePtr, uint64_t safeBytes,
                                      uint64_t *bitsBuffer0, uint64_t *bitsBuffer1, 
                                      uint64_t *bitsBuffer2, uint64_t *bitsBuffer3) {

    vec256i_t *vecPtr = (vec256i_t *)(bytePtr);
    vec256i_t vec = _mm256_loadu_si256(vecPtr);
    bitsBuffer0 = _mm256_movemask_epi8(vec);

    if (safeBytes < 32 * 2) return 32 * 1;
    vec =  _mm256_loadu_si256(vecPtr + 1);
    bitsTemp = _mm256_movemask_epi8(vec) << 32;
    bitsBuffer0 = bitsTemp | bitsBuffer0;

    if (safeBytes < 32 * 3) return 32 * 2;
    vec =  _mm256_loadu_si256(vecPtr + 2);
    bitsBuffer1 = _mm256_movemask_epi8(vec);

    if (safeBytes < 32 * 4) return 32 * 3;
    vec =  _mm256_loadu_si256(vecPtr + 3);
    bitsTemp = _mm256_movemask_epi8(vec) << 32;
    bitsBuffer1 = bitsTemp | bitsBuffer1;

    if (safeBytes < 32 * 5) return 32 * 4;
    vec =  _mm256_loadu_si256(vecPtr + 4);
    bitsBuffer2 = _mm256_movemask_epi8(vec);

    if (safeBytes < 32 * 6) return 32 * 5;
    vec =  _mm256_loadu_si256(vecPtr + 5);
    bitsTemp = _mm256_movemask_epi8(vec) << 32;
    bitsBuffer2 = bitsTemp | bitsBuffer2;

    if (safeBytes < 32 * 7) return 32 * 6;
    vec =  _mm256_loadu_si256(vecPtr + 6);
    bitsBuffer3 = _mm256_movemask_epi8(vec);

    if (safeBytes < 32 * 8) return 32 * 7;
    vec =  _mm256_loadu_si256(vecPtr + 7);
    bitsTemp = _mm256_movemask_epi8(vec) << 32;
    bitsBuffer3 = bitsTemp | bitsBuffer3;

    return 32 * 8;
}

// Other length specification options:
// uint8_t *vbyte_decode_branchless_avx2(uint8_t *inPtr, uint64_t intsToWrite, uint32_t *outPtr) 
// uint8_t *vbyte_decode_branchless_avx2(uint8_t *inPtr, uint32_t *outPtr)  // check for end pattern

uint8_t *vbyte_decode_branchless_avx2(uint8_t *inPtr, uint64_t bytesToRead, uint32_t *outPtr) {
    const uint64_t reloadThreshold = 12;
    vec256i_t prev = _mm256_setzero_si256();
    uint64_t safeBytes = bytesToRead - 32;  // last safe point to start a 32B read
    uint64_t consumed = 0;

    if (safeBytes < 32) return _finishScalar(inPtr);
    uint64_t bitsBuffer0, bitsBuffer1, bitsBuffer2, bitsBuffer3;
    uint64_t bytesRead = _reloadBuffer(inPtr, safeBytes, bitsRemaining,
                                       &bitsBuffer0, &bitsBuffer1,
                                       &bitsBuffer2, &bitsBuffer3);
    uint64_t bitsRemaining = bytesRead;  // expect 256 unless input is short
    inPtr += bytesRead;         // adjusted by bitsRemaining at reload
    safeBytes -= bytesRead;     // adjusted by bitsRemaining at reload

    while (1) {
        IACA_START;

        // FIXME: where should this shift go? 
        bitsBuffer <<= consumed;  // shift out the bits that were used

        uint64_t lookup = bitsBuffer & LOOKUP_MASK_BITS;  
        uint64_t consumed = vbyte_lookup_table[lookup].consumed;
        uint64_t index =  vbyte_lookup_table[lookup].index;

        uint64_t validInts = vbyte_valid_table[index]; 

        // FIXME: two different inPtr's --- buffer and data
        vec128i_t raw = _mm_loadu_si128((vec128i_t *)inPtr);

        // check for possible refill immediately after previous loads are issued
        
        bitsRemaining -= consumed;
        if (bitsRemaining < reloadThreshold) {
            inPtr -= bitsRemaining;     // remaining bits will be reloaded
            safeBytes += bitsRemaining; // space increases by bitsRemaining
            // if it's unsafe to read a full vector, or if the last reload hit the end...
            if (safeBytes < 32 || bytesRead < 256) return _finishScalar(inPtr);
            bytesRead = _reloadBuffer(inPtr, safeBytes, numRemaining,
                                      &bitsBuffer0, &bitsBuffer1,
                                      &bitsBuffer2, &bitsBuffer3);
        }

        vec256i_t data = _shuffleRaw(raw, index);
        
        data = _shrinkGaps(data);

        data = _inverseDelta(data, prev);

        _mm256_storeu_si256((vec256i_t *)outPtr, data);

        outPtr += validInts;

        // create vec from last int written as previous value for next iteration
        prev = _mm256_broadcastd_epi32(*(vec128i_t *)(outPtr - 1));
        IACA_END;
    }

    return inPtr;
}


    
    

    
    

    
    
