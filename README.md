# Layout

* `include` - header files
* `ciphers` - crypto algorithms
  * `rijndael` - AES standard with GCM
  * `chacha20` - ChaCha20 cipher with Poly1305
* `tests` - test vectors for ciphers
* `bench` - benchmarking scripts

A comparison of AES-256-GCM and ChaCha20-Poly1305 AEAD algorithms on contemporary hardware. Both constructions at their core use symmetric ciphers; AES is a block cipher and ChaCha20 is a stream cipher. Since AES-256-GCM uses AES in counter mode, it is effectively a stream cipher. So really we are comparing two AEAD stream cipher algorithms.

Performance benchmarks are done on a 64-bit x86 platform using FELICS which has been modified to use AEAD forms of encryption. A brief decription of the test machine can be found below.

A more detailed description of the performance of the algorithms can be found in the full report.