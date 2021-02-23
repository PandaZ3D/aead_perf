/* header
* fips compliant?
* only implements encryption
*/

#include <stdint.h>

/******************************************
 *             AES CRYPTO API             * 
 ******************************************/
#ifndef __AES_H__
#define __AES_H__

/* 
 * Creates key schedule from a single 256 byte key
 * to be used in every round of encryption. In other
 * words, generates round keys.
 */
void aes_key_expansion(const uint8_t secret_key[], uint32_t key_schedule[]);

// encrypts a single 128-bit block with key schedule
void aes_encryption(const uint8_t plain_text[], uint8_t cipher_text[], const uint32_t key_schedule[]);

#endif
