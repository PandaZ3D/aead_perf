/* 
 * Author: Allen Aboytes 
 * 
 * Details: 
 * An implementation of ChaCha20 based on RFC-8439. The specification can 
 * be found below:
 *   https://tools.ietf.org/html/rfc8439
 * 
 * A background on the Salsa20 encryption function is helpful
 * since ChaCha is a modification of the original design to
 * provide better diffusion. ChaCha at its core is essentially
 * a hash function (snuffle) operated in counter mode to encrypt
 * data. The cipher design can be found here:
 *   https://cr.yp.to/chacha/chacha-20080128.pdf
 *   https://cr.yp.to/snuffle/salsafamily-20071225.pdf
 *
 * This implementation passes all test vectors specified in RFC-8439.
 * 
 * Disclaimer: 
 * Source provided "as is" with no gaurentees. Created for
 * eductational purposes.
 */

#include <string.h>

// // use sse2 or sse3
#include <immintrin.h>
#include <stdint.h>

// #include "chacha20.h"

#include <stdio.h>
void print_state(uint32_t state[], int words) {
    for(int i = 0; i < words; i++) {
        printf("%08x ", state[i]);
        if((i+1) % 4 == 0) printf("\n");
    }
    printf("\n");
}

void print_vec_state(__m256i state[]) {
    for(int row = 0; row < 4; row++) {
        // print individual numbers
        printf("%08x %08x %08x %08x\n",
            _mm256_extract_epi32(state[row], 0), 
            _mm256_extract_epi32(state[row], 1), 
            _mm256_extract_epi32(state[row], 2), 
            _mm256_extract_epi32(state[row], 3));
    }
    printf("\n");

    for(int row = 0; row < 4; row++) {
        // print individual numbers
        printf("%08x %08x %08x %08x\n",
            _mm256_extract_epi32(state[row], 4), 
            _mm256_extract_epi32(state[row], 5), 
            _mm256_extract_epi32(state[row], 6), 
            _mm256_extract_epi32(state[row], 7));
    }
    printf("\n");
}

/******************************************
 *          MACROS and CONSTANTS          * 
 ******************************************/

// salsa/chacha has a block size of 64 bytes
#define CHACHA20_BLKSZ 64
#define CHACHA20_WORDS 16 // in 32-bit words

#define N_ROUNDS 20

// load word from a byte array in little-endian
#define _load_word32_le(b) \
    ((b[0])     | \
    (b[1] << 8) | \
    (b[2] << 16) | \
    (b[3] << 24))

// store word into byte array from 32-bit word
#define _store_word32_le(b, w) \
    (b[0]) = (w); \
    (b[1]) = (w >> 8); \
    (b[2]) = (w >> 16); \
    (b[3]) = (w >> 24);

#define _mm_rotl_256(x, shift) \
    (_mm256_or_si256( \
      _mm256_slli_si256(x, (shift)>>3), \
      _mm256_srli_si256(x, (128 - shift)>>3)))

#define _mm_rotr_256(x, shift) \
    (_mm256_or_si256( \
      _mm256_slli_si256(x, (128 - shift)>>3), \
      _mm256_srli_si256(x, (shift>>3))))

// constant used in expansion for hash state
static const char sigma_const[] = {'e','x','p','a','n','d',
    ' ','3','2','-','b','y','t','e',' ','k'};

/******************************************
 *        CORE ChaCha20 FUNCTIONS         * 
 ******************************************/

/* for qouble round, rotate 4 32-bit words in vector */
static inline __m256i rotl_256_u32(__m256i in, int shift) {
    if(shift <= 0)
        return in;
    return _mm256_or_si256(_mm256_slli_epi32(in, shift), \
        _mm256_srli_epi32(in, 32 - shift));
}

/* performs all 4 quarter round transformations in parallel
for row and column/diagonal round transformations */

static inline void chacha20_double_round_256(__m256i * x) {
    /* row round portion of double round */
    // v0 += v1; v3 ^= v0; v3 <<<= (16, 16, 16, 16); 
    x[0] = _mm256_add_epi32(x[0], x[1]);
    x[3] = _mm256_xor_si256(x[3], x[0]);
    x[3] = rotl_256_u32(x[3], 16);

    // v2 += v3; v1 ^= v2; v1 <<<= (12, 12, 12, 12); 
    x[2] = _mm256_add_epi32(x[2], x[3]);
    x[1] = _mm256_xor_si256(x[1], x[2]);
    x[1] = rotl_256_u32(x[1], 12);

    // v0+=v1;v3^=v0;v3<<<=(8, 8, 8, 8); 
    x[0] = _mm256_add_epi32(x[0], x[1]);
    x[3] = _mm256_xor_si256(x[3], x[0]);
    x[3] = rotl_256_u32(x[3], 8);

    // v2+=v3;v1^=v2; v1<<<=(7, 7, 7, 7); 
    x[2] = _mm256_add_epi32(x[2], x[3]);
    x[1] = _mm256_xor_si256(x[1], x[2]);
    x[1] = rotl_256_u32(x[1], 7);

    printf("%s: row transform\n", __func__);
    print_vec_state(x);
    // v1 >>>= 32; v2 >>>= 64; v3 >>>= 96;
    x[1] = _mm_rotr_256(x[1], 32);
    x[2] = _mm_rotr_256(x[2], 64);
    x[3] = _mm_rotr_256(x[3], 96);

    /* column round portion of double round */
    // v0 += v1; v3 ^= v0; v3 <<<= (16, 16, 16, 16); 
    x[0] = _mm256_add_epi32(x[0], x[1]);
    x[3] = _mm256_xor_si256(x[3], x[0]);
    x[3] = rotl_256_u32(x[3], 16);

    // v2 += v3; v1 ^= v2; v1 <<<= (12, 12, 12, 12); 
    x[2] = _mm256_add_epi32(x[2], x[3]);
    x[1] = _mm256_xor_si256(x[1], x[2]);
    x[1] = rotl_256_u32(x[1], 12);

    // v0+=v1;v3^=v0;v3<<<=(8, 8, 8, 8); 
    x[0] = _mm256_add_epi32(x[0], x[1]);
    x[3] = _mm256_xor_si256(x[3], x[0]);
    x[3] = rotl_256_u32(x[3], 8);

    // v2+=v3;v1^=v2; v1<<<=(7, 7, 7, 7); 
    x[2] = _mm256_add_epi32(x[2], x[3]);
    x[1] = _mm256_xor_si256(x[1], x[2]);
    x[1] = rotl_256_u32(x[1], 7);

    // v1 <<<= 32; v2 <<<= 64; v3 <<<= 96;
    x[1] = _mm_rotl_256(x[1], 32);
    x[2] = _mm_rotl_256(x[2], 64);
    x[3] = _mm_rotl_256(x[3], 96);

    printf("%s: col transform\n", __func__);
    print_vec_state(x);


}

// x is the input state, y is the srialized output of the hash
/* the core hash/compression function of chacha */
/* applies quarter round transformations */
void chacha20_hash_256(const __m256i x[], uint8_t y[]) {
    __m256i z[4]; // working state
    // copy in data (already in little endian)
    z[0] = _mm256_lddqu_si256(x);
    z[1] = _mm256_lddqu_si256(x + 1);
    z[2] = _mm256_lddqu_si256(x + 2);
    z[3] = _mm256_lddqu_si256(x + 3);
    
    // perform 10 double rounds = 20 rounds
    for(int i = 0; i < N_ROUNDS; i+=2) {
        printf("%s: vector state round [%d,%d]\n", __func__, i+1, i+2);
        print_vec_state(z);
        chacha20_double_round_256(z);
    }
    // print_state(z, 16);

    // addition with original input
    int off = 0;
    for(int i = 0; i < 4; i++) {
        z[i] = _mm256_add_epi32(z[i], x[i]);
        // store result and desearialize from little endian format
        _store_word32_le((y + off), \
            (_mm256_extract_epi32(z[i], 0)));
        _store_word32_le((y + off + 4), \
                (_mm256_extract_epi32(z[i], 1)));
        _store_word32_le((y + off + 8), \
            (_mm256_extract_epi32(z[i], 2)));
        _store_word32_le((y + off + 12), \
            (_mm256_extract_epi32(z[i], 3)));
        _store_word32_le((y + off + 64), \
            (_mm256_extract_epi32(z[i], 4)));
        _store_word32_le((y + off + 68), \
                (_mm256_extract_epi32(z[i], 5)));
        _store_word32_le((y + off + 72), \
            (_mm256_extract_epi32(z[i], 6)));
        _store_word32_le((y + off + 76), \
            (_mm256_extract_epi32(z[i], 7)));
        off += 32;
    }
    printf("%s: vector keystream\n", __func__);
    print_vec_state(z);
}

// input assumed to be 16 bytes (counter || nonce)
// output assumed to be 64-bytes
// key is 32 bytes
// setup function uses 128-bit vectors for the rest of the cipher
void chacha20_expansion_256(const uint8_t key[], uint8_t input[], uint8_t output[]) {
    // construct state matrix (in little endian)
    __attribute__((aligned(16))) \
    uint32_t state[CHACHA20_WORDS * 2];

    // initialize first state
    // copy in constants (sigma for 32-byte key)
    state[0] = _load_word32_le(sigma_const);
    state[1] = _load_word32_le((sigma_const + 4));
    state[2] = _load_word32_le((sigma_const + 8));
    state[3] = _load_word32_le((sigma_const + 12));
    // copy in key material (k0 and k1)
    state[4] = _load_word32_le(key);
    state[5] = _load_word32_le((key + 4));
    state[6] = _load_word32_le((key + 8));
    state[7] = _load_word32_le((key + 12));
    state[8] = _load_word32_le((key + 16));
    state[9] = _load_word32_le((key + 20));
    state[10] = _load_word32_le((key + 24));
    state[11] = _load_word32_le((key + 28));
    // copy in input (counter || nonce)
    state[12] = _load_word32_le(input);
    state[13] = _load_word32_le((input + 4));
    state[14] = _load_word32_le((input + 8));
    state[15] = _load_word32_le((input + 12));

    // copy the same state data
    memcpy(state + 16, state, 64);

    // increment counter value
    input[0] += 1; // counter is a single 32-bit value
    // incrment same value in second state block
    state[28] += 1;

    printf("%s: first block\n", __func__);
    print_state(state, 16);
    printf("%s: second block\n", __func__);
    print_state(state + 16, 16);
    // copy in constants (sigma for 32-byte key)
    // state[16] = _load_word32_le(sigma_const);
    // state[17] = _load_word32_le((sigma_const + 4));
    // state[18] = _load_word32_le((sigma_const + 8));
    // state[19] = _load_word32_le((sigma_const + 12));
    // // copy in key material (k0 and k1)
    // state[20] = _load_word32_le(key);
    // state[21] = _load_word32_le((key + 4));
    // state[22] = _load_word32_le((key + 8));
    // state[23] = _load_word32_le((key + 12));
    // state[24] = _load_word32_le((key + 16));
    // state[25] = _load_word32_le((key + 20));
    // state[26] = _load_word32_le((key + 24));
    // state[27] = _load_word32_le((key + 28));
    // // copy in input (counter || nonce)
    // state[28] = _load_word32_le(input);
    // state[29] = _load_word32_le((input + 4));
    // state[30] = _load_word32_le((input + 8));
    // state[31] = _load_word32_le((input + 12));

    // load both states into 256-bit vector arrays
    __m256i state_256[4] = {
        _mm256_loadu2_m128i((__m128i *) (state + 16), (__m128i *) state),
        _mm256_loadu2_m128i((__m128i *) (state + 20), (__m128i *) (state + 4)),
        _mm256_loadu2_m128i((__m128i *) (state + 24), (__m128i *) (state + 8)),
        _mm256_loadu2_m128i((__m128i *) (state + 28), (__m128i *) (state + 12))
    };

    printf("%s: printing vector state\n", __func__);
    print_vec_state(state_256);
    // use the salsa20 hash algorithm (modified quarter round)
    chacha20_hash_256(state_256, output);
}

/******************************************
 *          ChaCha20 CRYPTO API           * 
 ******************************************/

// nonce is 64 bits, so is counter
// following specification in RFC 7539
//  96-bit nonce, 32-bit counter
void chacha20_encryption_256 (
    const uint8_t in[], uint8_t out[],
    const uint8_t k[], const uint8_t n[], const uint32_t c, int inlen) {
        int i, j, off = 0, dbl_blocks, bytes_left;
        
        uint8_t keystream[128];
        uint8_t iv[16];
        // uint32_t count = c;

        // copy in nonce and counter
        _store_word32_le(iv, c);
        memcpy(iv + 4, n, 12);

        // process 2 blocks at a time in cipher
        bytes_left = inlen % 128;
        dbl_blocks = inlen / 128;

        printf("%s: dbl(%d blocks) left(%d bytes)\n",
            __func__, dbl_blocks, bytes_left);
        
        for (i = 0; i < dbl_blocks; i++) {
            printf("enter double block");
            // generate key stream for 2 blocks
            // count incremented in expansion
            chacha20_expansion_256(k, iv, keystream);
            // print_block(keystream);
            // encrypt plain text
            for(j = 0; j < 128; j++) {
                out[off + j] = in[off + j] ^ keystream[j];
            }
            off += 128;
        }

        if(bytes_left > 0) {
            printf("leftover bytes [0..127]\n");
            // generate key stream
            chacha20_expansion_256(k, iv, keystream);
            // print_block(keystream);
            // leftover blocks
            for(i = 0; i < bytes_left; i++) {
                printf("%s: perform xor off=%d i=%d out=%02x\n",
                    __func__, off, i, out[i]);
                out[off + i] = in[off + i] ^ keystream[i];
            }
        }
}


#define _CHACHA20_AVX_MAIN_
#ifdef _CHACHA20_AVX_MAIN_

#include <stdio.h>
#include <string.h>

// #include "chacha20.h"

// store word into byte array from 32-bit word
// #define _store_word32_le(b, w) \
//     (b[0]) = (w); \
//     (b[1]) = (w >> 8); \
//     (b[2]) = (w >> 16); \
//     (b[3]) = (w >> 24);

// void print_state(uint32_t state[], int words) {
//     for(int i = 0; i < words; i++) {
//         printf("%08x ", state[i]);
//         if((i+1) % 4 == 0) printf("\n");
//     }
//     printf("\n");
// }

// void zero_init(uint32_t y[], int len) {
//     for(int i = 0; i < len; i++) {
//         y[i] = 0;
//     }
// }

int main() {
    // test chacha block function (expansion/hash)
    uint8_t key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };

    uint8_t nonce[] = {
        0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 
        0x00, 0x4a, 0x00, 0x00, 0x00, 0x00
    };

    uint32_t counter = 1;

    uint8_t iv[16]; //z[64];

    _store_word32_le(iv, counter);
    memcpy(iv + 4, nonce, 12);

    // test chacha encryption
    nonce[3] = 0;
    uint8_t pt[] = {
        0x4c, 0x61, 0x64, 0x69, 0x65, 0x73, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x47, 0x65, 0x6e, 0x74, 0x6c,
        0x65, 0x6d, 0x65, 0x6e, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x61, 0x73,
        0x73, 0x20, 0x6f, 0x66, 0x20, 0x27, 0x39, 0x39, 0x3a, 0x20, 0x49, 0x66, 0x20, 0x49, 0x20, 0x63,
        0x6f, 0x75, 0x6c, 0x64, 0x20, 0x6f, 0x66, 0x66, 0x65, 0x72, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x6f,
        0x6e, 0x6c, 0x79, 0x20, 0x6f, 0x6e, 0x65, 0x20, 0x74, 0x69, 0x70, 0x20, 0x66, 0x6f, 0x72, 0x20,
        0x74, 0x68, 0x65, 0x20, 0x66, 0x75, 0x74, 0x75, 0x72, 0x65, 0x2c, 0x20, 0x73, 0x75, 0x6e, 0x73,
        0x63, 0x72, 0x65, 0x65, 0x6e, 0x20, 0x77, 0x6f, 0x75, 0x6c, 0x64, 0x20, 0x62, 0x65, 0x20, 0x69,
        0x74, 0x2e
    };
    int pt_len = 114;
    uint8_t ct[pt_len];

    printf("chacha encryption test (ptlen = %d)\n", pt_len);
    chacha20_encryption_256(pt, ct, key, nonce, counter, pt_len);
    for(int i = 0; i < pt_len; i++) {
        printf("%02x ", ct[i]);
        if((i+1) % 16 == 0) printf("\n");
    }
    printf("\n");

    uint8_t expected_ct[] = {
        0x6e, 0x2e, 0x35, 0x9a, 0x25, 0x68, 0xf9, 0x80, 0x41, 0xba, 0x07, 0x28, 0xdd, 0x0d, 0x69, 0x81, 
        0xe9, 0x7e, 0x7a, 0xec, 0x1d, 0x43, 0x60, 0xc2, 0x0a, 0x27, 0xaf, 0xcc, 0xfd, 0x9f, 0xae, 0x0b,
        0xf9, 0x1b, 0x65, 0xc5, 0x52, 0x47, 0x33, 0xab, 0x8f, 0x59, 0x3d, 0xab, 0xcd, 0x62, 0xb3, 0x57,
        0x16, 0x39, 0xd6, 0x24, 0xe6, 0x51, 0x52, 0xab, 0x8f, 0x53, 0x0c, 0x35, 0x9f, 0x08, 0x61, 0xd8,
        0x07, 0xca, 0x0d, 0xbf, 0x50, 0x0d, 0x6a, 0x61, 0x56, 0xa3, 0x8e, 0x08, 0x8a, 0x22, 0xb6, 0x5e,
        0x52, 0xbc, 0x51, 0x4d, 0x16, 0xcc, 0xf8, 0x06, 0x81, 0x8c, 0xe9, 0x1a, 0xb7, 0x79, 0x37, 0x36,
        0x5a, 0xf9, 0x0b, 0xbf, 0x74, 0xa3, 0x5b, 0xe6, 0xb4, 0x0b, 0x8e, 0xed, 0xf2, 0x78, 0x5e, 0x42, 
        0x87, 0x4d
    };

    for(int i = 0; i < pt_len; i++) {
        if (ct[i] != expected_ct[i]) {
            return -1;
        }
    }
}
#endif
