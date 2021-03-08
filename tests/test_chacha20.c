#include <stdio.h>
#include <string.h>

#include "chacha20.h"

// store word into byte array from 32-bit word
#define _store_word32_le(b, w) \
    (b[0]) = (w); \
    (b[1]) = (w >> 8); \
    (b[2]) = (w >> 16); \
    (b[3]) = (w >> 24);

void print_state(uint32_t state[], int words) {
    for(int i = 0; i < words; i++) {
        printf("%08x ", state[i]);
        if((i+1) % 4 == 0) printf("\n");
    }
    printf("\n");
}

void zero_init(uint32_t y[], int len) {
    for(int i = 0; i < len; i++) {
        y[i] = 0;
    }
}

int main() {
    // // test quarter round
    // uint32_t x[] = {
    //     0x11111111, // a
    //     0x01020304, // b
    //     0x9b8d6f43, // c
    //     0x01234567  // d
    // };
    // print_state(x, 4);
    // chacha_quarter_round(x, 0, 1, 2, 3);
    // print_state(x, 4);

    // // test quarter round on chacha state
    // uint32_t sample_state[] = {
    //     0x879531e0, 0xc5ecf37d, 0x516461b1, 0xc9a62f8a,
    //     0x44c20ef3, 0x3390af7f, 0xd9fc690b, 0x2a5f714c,
    //     0x53372767, 0xb00a5631, 0x974c541a, 0x359e9963,
    //     0x5c971061, 0x3d631689, 0x2098d9d6, 0x91dbd320
    // };

    // print_state(sample_state, 16);
    // chacha_quarter_round(sample_state, 2, 7, 8, 13);
    // print_state(sample_state, 16);

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

    // chacha20_expansion(key, iv, z);

    // for(int i = 0; i < 64; i++) {
    //     printf("%02x ", z[i]);
    //     if((i+1) % 16 == 0) printf("\n");
    // }

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
    chacha20_encryption(pt, ct, key, nonce, counter, pt_len);
    for(int i = 0; i < pt_len; i++) {
        printf("%02x ", ct[i]);
        if((i+1) % 16 == 0) printf("\n");
    }
    printf("\n");
}