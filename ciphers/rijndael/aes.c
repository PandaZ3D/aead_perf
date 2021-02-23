/* header
* fips compliant?
* only implements encryption
*/

#include <string.h>

#include "aes.h"

/* AES constants
 * Nb = 4 (numbe of 32-bit words in state)
*/
/******************************************
 *          MACROS and CONSTANTS          * 
 ******************************************/
#define BLK_BYTES 16  // block size in bytes
#define N_ROWS    4   // number of rows in state
#define N_COLUMNS 4   // number of columns in state
#define N_ROUNDS 14   // number of rounds in AES-256

// AES state indexing functions
#define MATRIX(s, r, c) (s[c][r])
#define COLARR(col, r) ((uint8_t* ) &col)[r]

// round constant words for key expansion
static const uint32_t rcon[] = {
  0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000,
  0x20000000, 0x40000000, 0x80000000, 0x1B000000, 0x36000000,
  0x6C000000, 0xD8000000, 0xAB000000, 0x4D000000, 0x9A000000
};

// substitution boxes used in SubBytes and SubWord
static const uint8_t s_box[16][16] = {
  {0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76},
	{0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0},
	{0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15},
	{0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75},
	{0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84},
	{0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF},
	{0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8},
	{0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2},
	{0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73},
	{0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB},
	{0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79},
	{0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08},
	{0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A},
	{0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E},
	{0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF},
	{0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16}
};

// only indexes are 0x02 and 0x03 since we implement
// encryptino only. For any other coefficeitns we would need
// fully calculated tables. An alternative and more efficient
// method is using algo-log tables (x2 256 tables)
#define GF_8(c, b) gf_8_mult[c ^ 0x02][b]

// could use precomputed tables 
/* first index refers to 0x02 and second refers to 0x03 */
static const uint8_t gf_8_mult[2][256] = {
  /* multiplication by 0x02 */
  {0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E, 
  0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E, 
  0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E, 
  0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E, 
  0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E, 
  0xA0, 0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0xBE, 
  0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE, 
  0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE, 0xF0, 0xF2, 0xF4, 0xF6, 0xF8, 0xFA, 0xFC, 0xFE, 
  0x1B, 0x19, 0x1F, 0x1D, 0x13, 0x11, 0x17, 0x15, 0x0B, 0x09, 0x0F, 0x0D, 0x03, 0x01, 0x07, 0x05, 
  0x3B, 0x39, 0x3F, 0x3D, 0x33, 0x31, 0x37, 0x35, 0x2B, 0x29, 0x2F, 0x2D, 0x23, 0x21, 0x27, 0x25, 
  0x5B, 0x59, 0x5F, 0x5D, 0x53, 0x51, 0x57, 0x55, 0x4B, 0x49, 0x4F, 0x4D, 0x43, 0x41, 0x47, 0x45, 
  0x7B, 0x79, 0x7F, 0x7D, 0x73, 0x71, 0x77, 0x75, 0x6B, 0x69, 0x6F, 0x6D, 0x63, 0x61, 0x67, 0x65, 
  0x9B, 0x99, 0x9F, 0x9D, 0x93, 0x91, 0x97, 0x95, 0x8B, 0x89, 0x8F, 0x8D, 0x83, 0x81, 0x87, 0x85, 
  0xBB, 0xB9, 0xBF, 0xBD, 0xB3, 0xB1, 0xB7, 0xB5, 0xAB, 0xA9, 0xAF, 0xAD, 0xA3, 0xA1, 0xA7, 0xA5, 
  0xDB, 0xD9, 0xDF, 0xDD, 0xD3, 0xD1, 0xD7, 0xD5, 0xCB, 0xC9, 0xCF, 0xCD, 0xC3, 0xC1, 0xC7, 0xC5, 
  0xFB, 0xF9, 0xFF, 0xFD, 0xF3, 0xF1, 0xF7, 0xF5, 0xEB, 0xE9, 0xEF, 0xED, 0xE3, 0xE1, 0xE7, 0xE5},
  /* multiplication by 0x03 */  
  {0x00, 0x03, 0x06, 0x05, 0x0C, 0x0F, 0x0A, 0x09, 0x18, 0x1B, 0x1E, 0x1D, 0x14, 0x17, 0x12, 0x11, 
  0x30, 0x33, 0x36, 0x35, 0x3C, 0x3F, 0x3A, 0x39, 0x28, 0x2B, 0x2E, 0x2D, 0x24, 0x27, 0x22, 0x21, 
  0x60, 0x63, 0x66, 0x65, 0x6C, 0x6F, 0x6A, 0x69, 0x78, 0x7B, 0x7E, 0x7D, 0x74, 0x77, 0x72, 0x71, 
  0x50, 0x53, 0x56, 0x55, 0x5C, 0x5F, 0x5A, 0x59, 0x48, 0x4B, 0x4E, 0x4D, 0x44, 0x47, 0x42, 0x41, 
  0xC0, 0xC3, 0xC6, 0xC5, 0xCC, 0xCF, 0xCA, 0xC9, 0xD8, 0xDB, 0xDE, 0xDD, 0xD4, 0xD7, 0xD2, 0xD1, 
  0xF0, 0xF3, 0xF6, 0xF5, 0xFC, 0xFF, 0xFA, 0xF9, 0xE8, 0xEB, 0xEE, 0xED, 0xE4, 0xE7, 0xE2, 0xE1, 
  0xA0, 0xA3, 0xA6, 0xA5, 0xAC, 0xAF, 0xAA, 0xA9, 0xB8, 0xBB, 0xBE, 0xBD, 0xB4, 0xB7, 0xB2, 0xB1, 
  0x90, 0x93, 0x96, 0x95, 0x9C, 0x9F, 0x9A, 0x99, 0x88, 0x8B, 0x8E, 0x8D, 0x84, 0x87, 0x82, 0x81, 
  0x9B, 0x98, 0x9D, 0x9E, 0x97, 0x94, 0x91, 0x92, 0x83, 0x80, 0x85, 0x86, 0x8F, 0x8C, 0x89, 0x8A, 
  0xAB, 0xA8, 0xAD, 0xAE, 0xA7, 0xA4, 0xA1, 0xA2, 0xB3, 0xB0, 0xB5, 0xB6, 0xBF, 0xBC, 0xB9, 0xBA, 
  0xFB, 0xF8, 0xFD, 0xFE, 0xF7, 0xF4, 0xF1, 0xF2, 0xE3, 0xE0, 0xE5, 0xE6, 0xEF, 0xEC, 0xE9, 0xEA, 
  0xCB, 0xC8, 0xCD, 0xCE, 0xC7, 0xC4, 0xC1, 0xC2, 0xD3, 0xD0, 0xD5, 0xD6, 0xDF, 0xDC, 0xD9, 0xDA, 
  0x5B, 0x58, 0x5D, 0x5E, 0x57, 0x54, 0x51, 0x52, 0x43, 0x40, 0x45, 0x46, 0x4F, 0x4C, 0x49, 0x4A, 
  0x6B, 0x68, 0x6D, 0x6E, 0x67, 0x64, 0x61, 0x62, 0x73, 0x70, 0x75, 0x76, 0x7F, 0x7C, 0x79, 0x7A, 
  0x3B, 0x38, 0x3D, 0x3E, 0x37, 0x34, 0x31, 0x32, 0x23, 0x20, 0x25, 0x26, 0x2F, 0x2C, 0x29, 0x2A, 
  0x0B, 0x08, 0x0D, 0x0E, 0x07, 0x04, 0x01, 0x02, 0x13, 0x10, 0x15, 0x16, 0x1F, 0x1C, 0x19, 0x1A}
};
/******************************************
 *           CORE AES FUNCTIONS           * 
 ******************************************/

/********* Key Expansion Helper **********/

// takes 4 byte input word and applies s-box to it
// returns output word, does not modify input directly
uint32_t SubWord(uint32_t word) {
  uint32_t sub_word = word;
  uint8_t byte, *byte_arr = (uint8_t *) &sub_word;
  printf("SubWord: ");
  for (int i = 0; i < N_ROUNDS; i++) {
    byte = byte_arr[i];
    printf("%x ", byte);
    byte_arr[i] = s_box[byte >> 4][byte & 0xF];
  }
  printf("\n");
  return sub_word;
}

// left cyclic permutation on single word
// word is 32 bytes
// byte rotated by 1
uint32_t RotWord(uint32_t word) {
  return (word << 8) | (word >> 24);
}

/********** AES Round Runctions **********/

/* byte substitution using non-linear S-Boxes */
void SubBytes(uint8_t state [][4]) {
  uint8_t tmp;
  for (int c = 0; c < N_COLUMNS; c++) {
    for (int r = 0; r < N_ROWS; r++) {
      tmp = MATRIX(state, r, c);
      MATRIX(state, r, c) = s_box[(tmp >> 4)][tmp & 0xF];
    }
  }
}

// first row remains unchanged
// only last three rows shifted
void ShiftRows(uint8_t state [][4]) {
  uint8_t tmp;
  // for row in state
  for (int r = 1; r < N_ROWS; r++) {
    // save first byte in row
    tmp = MATRIX(state, r, 0);
    // shift over next 3 bytes in row
    for (int c = 0; c < N_COLUMNS - 1; c++) {
      MATRIX(state, r, c) = MATRIX(state, r, (c + r) % N_COLUMNS);
    }
    // wrap around first byte
    MATRIX(state, r, N_COLUMNS - 1) = tmp;
  }
}

void MixColumns(uint32_t state[]) {
    uint32_t col;
    // for each column in state
    for (int c = 0; c < N_COLUMNS; c++) {
      //   multiply by special matrix in GF(2^8)
      col = state[c];
      //   s0' = ({02} AND s0) XOR ({03} AND s1) XOR s2 XOR s3
      COLARR(state[c], 0) = GF_8(0x02, COLARR(col, 0)) ^ GF_8(0x03, COLARR(col, 1))
        ^ COLARR(col, 2) ^ COLARR(col, 3);
      //   s1' = s0 XOR ({02} AND s1) XOR ({03} AND s2) XOR s3
      COLARR(state[c], 1) = (COLARR(col, 0)) ^ GF_8(0x02, COLARR(col, 1)) 
        ^ GF_8(0x03, COLARR(col, 1)) ^ COLARR(col, 3);
      //   s2' = s0 XOR s1 ({02} AND s2) XOR ({03} AND s3)
      COLARR(state[c], 2) = (COLARR(col, 0)) ^ (COLARR(col, 1)) 
        ^ GF_8(0x02, COLARR(col, 2)) ^ GF_8(0x03, COLARR(col, 3));
      //   s3' = ({03} AND s0) XOR s1 XOR s2 XOR ({02} AND s3)
      COLARR(state[c], 3) = GF_8(0x03, COLARR(col, 0)) ^ (COLARR(col, 1)) 
        ^ (COLARR(col, 2)) ^ GF_8(0x02, COLARR(col, 3));
  }
}


/* adds a round key to the transfrmed state */
// state interpreted as an array of columns (words)
// round key interpretes as an array of 32-bit words
// addition is done with XOR
void KeyAddition(uint32_t state[], const uint32_t round_key[]) {  
  for (int c = 0; c < N_COLUMNS; c++) {
    printf("state: %x round: %x xor: %x", state[c], round_key[c], state[c] ^ round_key[c]);
    state[c] = state[c] ^ round_key[c];
  }
}

/******************************************
 *             AES CRYPTO API             * 
 ******************************************/
#include <stdio.h>
/* 
 * Creates key schedule from a single 256 byte key
 * to be used in every round of encryption. In other
 * words, generates round keys.
 */
void aes_key_expansion(const uint8_t secret_key[], uint32_t key_schedule[]) {
  uint8_t i = 0, Nk = 8; // number of 32-bit words in cipher key (key length)
  uint32_t tmp = 0;

  do {
    key_schedule[i] = ((uint32_t *) secret_key)[i];
    //((secret_key[4 * i] << 24) | (secret_key[4 * i + 1] << 16) 
    //  | (secret_key[4 * i + 2] << 8) | (secret_key[4 * i + 3]));
  } while(++i < Nk);

  do {
    tmp = key_schedule[i-1];
    printf("i: %d tmp: %x ", i, tmp);
    if (i % Nk == 0) {
      printf("RotWord: %x SubWord: %x Rcon: %x ", RotWord(tmp), SubWord(tmp), rcon[i/Nk - 1]);
      tmp = SubWord(RotWord(tmp)) ^ rcon[i/Nk - 1];
      printf("XOR: %x ", tmp);
    } else if (Nk > 6 && i % Nk == 4) {
      tmp = SubWord(tmp);
    }
    printf("w[i-Nk]: %x ", key_schedule[i - Nk]);
    key_schedule[i] = key_schedule[i - Nk] ^ tmp;
    printf("w[i]: %x\n", key_schedule[i]);
      // ((uint8_t*)key_schedule + i)[0], ((uint8_t*)key_schedule + i)[1],
      // ((uint8_t*)key_schedule + i)[2], ((uint8_t*)key_schedule + i)[3]);
  } while (++i < N_COLUMNS * (N_ROUNDS+1));
} 

void print_state(uint8_t m[][4]) {
  for (int c = 0; c < N_COLUMNS; c++) {
    for (int r = 0; r < N_ROWS; r++) {
      printf("%x", MATRIX(m, r, c));
    }
  }
}

// encrypts a single 128-bit block with key schedule
void aes_encryption(const uint8_t plain_text[], uint8_t cipher_text[], const uint32_t key_schedule[]) {
  // AES state array
  union state {
    uint8_t matrix[N_COLUMNS][N_ROWS]; // matrix indexed by column and then row
    uint32_t col_arr[N_COLUMNS]; // interpreted as an array of 32-bit words
  } S;

  // copy bytes to state array
  memcpy((uint8_t *) S.col_arr, plain_text, BLK_BYTES);

  KeyAddition(S.col_arr, key_schedule); // add round key

  for (int r = 1; r < N_ROUNDS; r++) {
    printf("round[%02d]: ", r);
    print_state(S.matrix);
    SubBytes(S.matrix);
    ShiftRows(S.matrix);
    MixColumns(S.col_arr);
    KeyAddition(S.col_arr, key_schedule + (r * N_COLUMNS));
  }

  // last round N_ROUNDS, no mixColumns
  SubBytes(S.matrix);
  ShiftRows(S.matrix);
  KeyAddition(S.col_arr, key_schedule + (N_ROUNDS * N_COLUMNS));

  //copy out bytes from state array
  memcpy(cipher_text, (uint8_t *) S.col_arr, BLK_BYTES);
}
