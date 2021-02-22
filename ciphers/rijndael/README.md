AES with 128 block size and 256 bit key. FIPS-127 compliant.

# Functions

* AES round
  * byte substitution (s-box, inversion)
  * diffusion
    * shift rows (permutation)
    * mix column (multiplication)
  * key addition (addition)

# Notes

AES finite field has 256 elements, GF(2^8)
* field chosen because elements can be a single byte
* efficient for 8-bit microprocessors (S-Box/MixColumn)

GF(2^8) is an extension field
* has different rules for arithmetic
* elements represented by polynomials
* storage is 8-bit coefficient vector
  * coefficients in GF(2)
* AES irreducible polynomial
  * P(x) = x^8 + x^4 + x^3 + x + 1

Extension Field Arithmetic
* operations
  * addition/subtraction
  * multiplication
    * product divided by irreducible polynomial
    * often easier to individually reduce terms
  * inversion
    * for small fields lookup tables can be used
* polynomial operations the same
  * coefficent operations in GF(2)
  * XOR is multiplication
  * AND is addition