#ifndef __CHACHA_20_H__
#define __CHACHA_20_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void chacha20_encryption(
    const uint8_t in[], uint8_t out[],
    const uint8_t k[], const uint8_t n[], const uint32_t c, int inlen);

#ifdef __cplusplus
}
#endif

#endif
