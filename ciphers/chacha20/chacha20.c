/* 
 * Author: Allen Aboytes 
 * 
 * Details: 
 * An implementation of ChaCha20. The cipher design can be found here:
 *
 * A background on the Salsa20 encryption function is helpful
 * since ChaCha is based on it. It is essentially a hash function
 * operated in counter mode to encrypt data. The specification can 
 * be found below:
 *
 * Passes test vectors?
 * 
 * Disclaimer: 
 * Source provided "as is" with no gaurentees. Created for
 * eductational purposes.
 */

#include <string.h>

#include "chacha20.h"

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

// constant used in expansion for hash state
static const char sigma_const[] = {'e','x','p','a','n','d',
    ' ','3','2','-','b','y','t','e',' ','k'};

/******************************************
 *        CORE ChaCha20 FUNCTIONS         * 
 ******************************************/

/* quarter round helper */
static inline uint32_t rotl_32(uint32_t in, int shift) {
    if(shift < 0)
        return 0;
    return (in << shift) | (in >> (32 - shift));
}

/* quarter round function */
static inline void chacha_quarter_round(uint32_t x[], int a, int b, int c, int d) {
    x[a] += x[b]; x[d] ^= x[a]; x[d] = rotl_32(x[d], 16);
    x[c] += x[d]; x[b] ^= x[c]; x[b] = rotl_32(x[b], 12);
    x[a] += x[b]; x[d] ^= x[a]; x[d] = rotl_32(x[d], 8);
    x[c] += x[d]; x[b] ^= x[c]; x[b] = rotl_32(x[b], 7);
}

// x is the input state, y is the srialized output of the hash
/* the core hash/compression function of chacha */
/* applies quarter round transformations */
void chacha20_hash(uint32_t x[], uint8_t y[]) {
    uint32_t z[CHACHA20_WORDS]; // working state
    // copy in data (already in little endian)
    memcpy(z, x, 64);
    // print_state(z, 16);

    // perform 10 double rounds = 20 rounds
    for(int i = 0; i < N_ROUNDS; i+=2) {
        // first round modifies columns
        chacha_quarter_round(z, 0, 4, 8,12);
        chacha_quarter_round(z, 1, 5, 9,13);
        chacha_quarter_round(z, 2, 6,10,14);
        chacha_quarter_round(z, 3, 7,11,15);
        // second round modifies diagonals
        chacha_quarter_round(z, 0, 5,10,15);
        chacha_quarter_round(z, 1, 6,11,12);
        chacha_quarter_round(z, 2, 7, 8,13);
        chacha_quarter_round(z, 3, 4, 9,14);
    }
    // print_state(z, 16);

    // addition with original input
    for(int i = 0; i < CHACHA20_WORDS; i++) {
        z[i] += x[i];
        // store result
        _store_word32_le((y + (i * 4)), z[i]);
    }
    // print_state(z, 16);
}

// input assumed to be 16 bytes (counter || nonce)
// output assumed to be 64-bytes
// key is 32 bytes
void chacha20_expansion(const uint8_t key[], const uint8_t input[], uint8_t output[]) {
    // construct state matrix (in little endian)
    uint32_t state[CHACHA20_WORDS];
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
    
    // printf("copy in state\n");
    // print_state(state, 16);
    
    // use the salsa20 hash algorithm (modified quarter round)
    chacha20_hash(state, output);
}

/******************************************
 *          ChaCha20 CRYPTO API           * 
 ******************************************/

// nonce is 64 bits, so is counter
// following specification in RFC 7539
//  96-bit nonce, 32-bit counter
void chacha20_encryption(
    const uint8_t in[], uint8_t out[],
    const uint8_t k[], const uint8_t n[], const uint32_t c, int inlen) {
        int i, j, off = 0, whole_blocks;
        int bytes_left = inlen % 64;
        
        uint8_t keystream[64];
        uint8_t iv[16];
        uint32_t count = c;

        // copy in nonce and counter
        _store_word32_le(iv, count);
        memcpy(iv + 4, n, 12);

        // process whole blocks in cipher
        whole_blocks = inlen / 64;
        for (i = 0; i < whole_blocks; i++) {
            // generate key stream
            chacha20_expansion(k, iv, keystream);
            // print_block(keystream);
            // encrypt plain text
            for(j = 0; j < 64; j++) {
                out[off + j] = in[off + j] ^ keystream[j];
            }
            off += 64;
            // increment counter
            count += 1;
            // update iv
            _store_word32_le(iv, count);
        }

        if(bytes_left > 0) {
            // generate key stream
            chacha20_expansion(k, iv, keystream);
            // print_block(keystream);
            // leftover blocks
            for(i = 0; i < bytes_left; i++) {
                out[off + i] = in[off + i] ^ keystream[i];
            }
        }
}
