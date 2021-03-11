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


#include <string.h>

#include "aes.h"
#include "aes_ctr.h"

/* 
 * AES-256 constants:
 *  Nb = 4 (numbe of 32-bit words in state)
 *  Nk = 8 (number of 32-bit words in key)
 *  Nr = 14 (number of rounds in AES)
 */
/******************************************
 *          MACROS and CONSTANTS          * 
 ******************************************/
#define BLK_BYTES 16  // block size in bytes (128/8)

/******************************************
 *           CORE CTR FUNCTIONS           * 
 ******************************************/

/********* Increment Count Helper **********/
static inline void increment_count(uint8_t counter[]) {
  int i = BLK_BYTES;
  do {
    counter[i]++;
  } while(counter[i] == 0 && --i > 0); // change later for 32-bit
}

/******************************************
 *         AES CTR CRYPTO API             * 
 ******************************************/
/* 
 * Encrypts an arbitray sized message which is byte aligned. Requires the key_schedule
 * to be pre-computed and a nonce to be defined. The size of the output must also match
 * the size of the input.
 */

 // rfc 3686 specifies 4 byte (32-bit) nonce, 8 byte (64-bit) iv, 32-bit counter
 // 
void aes_ctr_encryption(const uint8_t plain_text[], uint8_t cipher_text[], 
  const uint32_t key_schedule[], //const uint32_t nonce, const uint64_t iv, 
  uint8_t counter[], int pt_length) 
{
  int whole_blocks = pt_length / BLK_BYTES;
  int bytes_left = pt_length % BLK_BYTES;
  int off = 0; // offset into pt/ct
  uint8_t key_stream[BLK_BYTES];

  /* encrypt whole blocks in order */
  for(int i = 0; i < whole_blocks; i++) {
    // generate keystream for encryption
    aes_encryption(counter, key_stream, key_schedule);
    // encrypt plain text with xor
    for(int j = off; j < off + BLK_BYTES; j++) {
      cipher_text[j] = plain_text[j] ^ key_stream[j - off];
    }
    // increment offset
    off += BLK_BYTES;
    // increment counter
    increment_count(counter);
  }

  /* check if there are bytes left */
  if(bytes_left > 0) {
    // generate keystream for encryption
    aes_encryption(counter, key_stream, key_schedule);
    // encrypt plain text with xor
    for(int j = off; j < off + bytes_left; j++) {
      cipher_text[j] = plain_text[j] ^ key_stream[j - off];
    }
  }
}
