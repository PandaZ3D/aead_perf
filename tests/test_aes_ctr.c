/* 
 * Test Vectors from NIST-SP 800-86A, 256-bit key
 */ 
#include <string.h>

#include "aes_ctr.h"
#include "common.h"

#define AES_KEY_SZ 256 // 256-bit key generates 14 rounds
#define AES_BLK_SZ 128
#define AES_256_NR 14
#define AES_256_NB 8

int main() {
    uint8_t secret_key[AES_KEY_SZ/8];
    uint32_t key_schedule[AES_256_NB * (AES_256_NR+1)];

    char * pt_str = "6bc1bee22e409f96e93d7e117393172a" \
        "ae2d8a571e03ac9c9eb76fac45af8e51" \
        "30c81c46a35ce411e5fbc1191a0a52ef" \
        "f69f2445df4f9b17ad2b417be66c3710";
    
    char * key_str = "603deb1015ca71be2b73aef0857d7781" \
        "1f352c073b6108d72d9810a30914dff4";

    char * ctr_str = "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

    size_t ptlen = strlen(pt_str);
    size_t keylen = strlen(key_str);
    size_t ctrlen = strlen(ctr_str);

    uint8_t plain_text[AES_BLK_SZ];
    uint8_t ctr_bytes[AES_BLK_SZ * 4];

    hex_to_bytes(key_str, secret_key, keylen);
    hex_to_bytes(pt_str, plain_text, ptlen);
    hex_to_bytes(ctr_bytes, ctr_str, ctrlen);

    // generate round keys
    aes_key_expansion(secret_key, key_schedule);

    // encrypt 4 128-bit blocks
    uint8_t cipher_text[AES_BLK_SZ];
    aes_ctr_encryption(plain_text, cipher_text, \
        key_schedule, ctr_bytes, ptlen);

    char * expected_ct = "601ec313775789a5b7a7f504bbf3d228" \
        "f443e3ca4d62b59aca84e990cacaf5c5" \
        "2b0930daa23de94ce87017ba2d84988d" \
        "dfc9c58db67aada613c2dd08457941a6";

    size_t ct_len = strlen(expected_ct);
    uint8_t out_ct[AES_BLK_SZ * 4];
    hex_to_bytes(expected_ct, out_ct, ct_len);

    for(int i = 0; i < AES_BLK_SZ/8; i++) {
        if (cipher_text[i] != out_ct[i]) {
            return -1;
        }
    }
    return 0;
}
