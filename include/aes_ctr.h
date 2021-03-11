/* 
 * Author: Allen Aboytes 
 * 
 * Details: 
 * An implementation of AES-256 in counter mode as specified
 * in NIST-SP 800-38A. The document can be found here:
 *   https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38a.pdf
 *
 * The AES core function is FIPS-197 compliant. The specification
 * created by NIST can be found here:
 *   https://csrc.nist.gov/csrc/media/publications/fips/197/final/documents/fips-197.pdf
 * 
 * Disclaimer: 
 * Source provided "as is" with no gaurentees. Created for
 * eductational purposes.
 */

#include <stdint.h>

/******************************************
 *             AES CRYPTO API             * 
 ******************************************/
#ifndef __AES_256_CTR_H__
#define __AES_256_CTR_H__

/* 
 * Encrypts an arbitray sized message which is byte aligned. Requires the key_schedule
 * to be pre-computed and a nonce to be defined. The size of the output must also match
 * the size of the input.
 */

 // rfc 3686 specifies 4 byte (32-bit) nonce, 8 byte (64-bit) iv, 32-bit counter
 // 
void aes_ctr_encryption(const uint8_t plain_text[], uint8_t cipher_text[], 
    const uint32_t key_schedule[], //const uint32_t nonce, const uint64_t iv, 
    uint8_t counter[], int pt_length);

#endif
