/* 
 * Author: Allen Aboytes 
 * 
 * Details: 
 * An implementation of AES-256. The full specification (FIPS-197)
 * created by NIST can be found here:
 *   https://csrc.nist.gov/csrc/media/publications/fips/197/final/documents/fips-197.pdf
 *
 * This source is not necessarily FIPS-197 compliant as
 * it only implements encryption. The intention is to use
 * it for AES-256-GCM which only needs the encryption function
 * due to the underlying counter mode of operation.
 *
 * This implementation does however, pass the test vectors
 * in FIPS-197 for the AES-256 algorithm.
 * 
 * Disclaimer: 
 * Source provided "as is" with no gaurentees. Created for
 * eductational purposes.
 */

#include <stdint.h>

/******************************************
 *             AES CRYPTO API             * 
 ******************************************/
#ifndef __AES_256_H__
#define __AES_256_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* 
 * Creates key schedule from a single 256 byte key
 * to be used in every round of encryption. In other
 * words, generates round keys for AES-256.
 */
void aes_key_expansion(const uint8_t secret_key[], uint32_t key_schedule[]);

/* Encrypts a single 128-bit block with pre-computed key schedule. */
void aes_encryption(const uint8_t plain_text[], uint8_t cipher_text[], const uint32_t key_schedule[]);

#ifdef __cplusplus
}
#endif

#endif
