#ifndef _TEST_COMMON_H_
#define _TEST_COMMON_H_

#include <stdint.h>

static inline /* https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int */
uint8_t ahex2int(char a, char b){

    a = (a <= '9') ? a - '0' : (a & 0x7) + 9;
    b = (b <= '9') ? b - '0' : (b & 0x7) + 9;

    return ((a << 4) + b) & 0xff;
}

static inline uint8_t* hex_to_bytes(char* hex, uint8_t* bytes, size_t str_len) {
  size_t i = 0, j = 0;
  while(j < str_len) {
    bytes[i] = ahex2int(hex[j], hex[j+1]);
    i++;
    j+=2;
  }
  return bytes;
}

#endif
