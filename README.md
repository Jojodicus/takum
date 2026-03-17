# Takum

https://arxiv.org/abs/2404.18603

## Linear Takum Encoding

Defined for bit-length `n >= 12`.
For shorter numbers: assume all trailing bits to be `0`

MSB->LSB

| sign | direction | regime | characteristic | fraction |
|---|---|---|---|---|
| `S` | `D` | `R` | `C` | `F` |
| `1` | `1` | `3` | `r` | `p` |

with

- `S`: sign bit
- `D`: direction bit
- `R`: regime bits (`uint` of length `3`)
- `r`: regime
  ```py
  7 - R if D == 0 else R # [0,7]
  ```
- `C`: characteristic bits (`uint` of length `r`)
- `c`: characteristic
  ```py
  -2**(r+1) + 1 + C if D == 0 else 2**r - 1 + C
  ```
- `p`: fraction bit count
  ```py
  n - r - 5 # [n-12, n-5]
  ```
- `F`: fraction bits (`uint` of length `p`)
- `f`: fraction
  ```py
  2**(-p) * F # [0,1)
  ```
- `e`: exponent
  ```py
  (-1)**S * (c + S) # [-255, 254]
  ```

Resulting takum:

```py
if D == R == C == F == 0:
    if S == 0:
        return 0
    else: # S == 1
        return NaR
else:
    return ((1 - 3*S) + f) * 2**e
```

Number range: `{0, NaR} u {-2**(-255), -2**255} u {2**(-255), 2**255}`
