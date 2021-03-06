/* 
 * Test Vectors from FIPS 197, 256-bit key
 */ 
#include <string.h>

#include "aes.h"
#include "common.h"

#define AES_KEY_SZ 256 // 256-bit key generates 14 rounds
#define AES_BLK_SZ 128
#define AES_256_NR 14
#define AES_256_NB 8

int main() {
    uint8_t secret_key[AES_KEY_SZ/8] = {
        0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
        0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
        0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
        0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4};
    uint32_t key_schedule[AES_256_NB * (AES_256_NR+1)];
    // uint32_t expected_rk[AES_256_NR];

    // example key for key schedule in FIPS 197 A.3
    aes_key_expansion(secret_key, key_schedule);

    char * pt_str = "00112233445566778899aabbccddeeff";
    char * key_str = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";

    size_t ptlen = strlen(pt_str);
    size_t keylen = strlen(key_str);

    uint8_t plain_text[AES_BLK_SZ];

    hex_to_bytes(key_str, secret_key, keylen);
    hex_to_bytes(pt_str, plain_text, ptlen);

    // generate round keys
    aes_key_expansion(secret_key, key_schedule);
    // encrypt 128-bit block
    uint8_t cipher_text[AES_BLK_SZ];
    aes_encryption(plain_text, cipher_text, key_schedule);

    char * expected_ct = "8ea2b7ca516745bfeafc49904b496089";
    size_t ct_len = strlen(expected_ct);
    uint8_t out_ct[AES_BLK_SZ];
    hex_to_bytes(expected_ct, out_ct, ct_len);

    for(int i = 0; i < AES_BLK_SZ/8; i++) {
        if (cipher_text[i] != out_ct[i]) {
            return -1;
        }
    }
    return 0;
}
