ChaCha20 stream cipher implementation. Reference from D. Bernstein implementation and specification as well as RFC 8439/7539.

# References
```
papers here
```

# Notes

256-bit stream cipher based on Salsa20.
* 20 rounds (i.e. Salsa20/20)
* simple operations (32-bit)
  * addition, xor, rotation
* rotations 1/3 of int. operation
  * 16 of each operation per round

Salsa20 
* encyption based off of a hash function
  * snuffle 2005
  * operated in counter mode
* 64-byte block
* operates on 32-bit words

ChaCha
* provides better diffusion
* just as parallel/vectorizable
