# Notes

* bits numbered from 0 to 128 for input
* bytes numbered increasing order too
* individual bits in a byte 7 to 0

# AES configuration

| key length (bits) | block size (bits) | rounds |
|-|
| 256 | 128 | 14 |

# AES Algorithm

## SubBytes

* s-box is a substitution table
* substitution determined by value of bytes in state
  * byte -> s_{x,y} -> S[x][y] = new value
  * first nible is x, second is y

## ShiftRows

* bytes in last 3 rows are cyclically shifted
  * left cricular shift amount depends on row
* entire bytes rotated (wraps around)
  * moves bytes to different columns

## MixColumn

* each column multiplied by fixed polynomial
* 