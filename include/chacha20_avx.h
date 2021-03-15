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

#ifndef __CHACHA_20_AVX_H__
#define __CHACHA_20_AVX_H__

// use sse2 or sse3
#include <immintrin.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void chacha20_encryption_256 (
    const uint8_t in[], uint8_t out[],
    const uint8_t k[], const uint8_t n[], const uint32_t c, int inlen);

#ifdef __cplusplus
}
#endif

#endif
