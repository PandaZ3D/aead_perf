#pragma once

#define SAMPLE_SIZE 9604
#define MAX_FILE_SIZE 64000

// bench name measurement, time (cycles or ns), size (bytes)
enum cipher_code {
  CHACHA20_BASE=0,
  CHACHA20_128=1,
  CHACHA20_256=2,
  CHACHA20_512=3,
  AES_CTR_BASE=4,
  AES_AESNI=5,
  CHACHA20_POLY_BASE=6, // include seperate vectorized chacha and poly??
  CHACHA20_POLY_128=7,
  CHACHA20_POLY_256=8,
  CHACHA20_POLY_512=9,
  AES_GCM_BASE=10,
  AES_GCM_AESNI=11 // further throughput??
};

enum measure_type {
  CYCLES=0,
  NANOSEC=1
};

static char* cipher_name[] = {
  "chacha_base",
  "chacha_128",
  "chacha_256",
  "aes_ctr_base",
  "aes_aesni",
  "chacha_poly_base",
  "chacha_poly_128",
  "chacha_poly_256",
  "chacha_poly_512",
  "aes_gcm_base",
  "aes_gcm_aesni"
};

static uint8_t nonce[] = {
    0x00, 0x00, 0x09, 0x00, 
    0x27, 0x20, 0xfd, 0x63,
    0x6f, 0x75, 0xde, 0x14, 
    0x00, 0x00, 0x00, 0x01
  };

static uint8_t key[] = {
    0xec, 0x3a, 0xce, 0x7d, 0x30, 0x57, 0xe0, 0xac, 
    0x7d, 0xef, 0xe1, 0x79, 0xe8, 0x88, 0x17, 0x47,
    0xcf, 0xfc, 0xd7, 0x42, 0xee, 0x11, 0x93, 0xae, 
    0x86, 0xf9, 0x22, 0x1b, 0x53, 0xca, 0xa7, 0xf8
};