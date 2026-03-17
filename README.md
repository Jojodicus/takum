# Takum

C++ header-only library for *takum arithmetic*, a real number format defined in [arXiv:2404.18603](https://arxiv.org/abs/2404.18603). More specifically: **linear takums**.

## Quick Start

Copy `src/takum.hpp` into your project (requires C++17) and include it:

```cpp
#include "takum.hpp"

using namespace takum;

takum16 x(1.5);
takum16 y(2.0);

std::cout << x + y;   // 3.5
std::cout << x * y;   // 3
std::cout << x / y;   // 0.75
```

## Types

| Alias | Width | Notes |
|---|---|---|
| `takum12` | 12 bits | minimum width |
| `takum16` | 16 bits | good general-purpose choice |
| `takum32` | 32 bits | highest precision |

Custom widths from 12 to 32 are also available via `Takum<N>`.

## API

```cpp
// Construction
takum16 a;                          // zero
takum16 b(3.14);                    // from double
takum16 c(static_cast<int64_t>(7)); // from integer
takum16 d = takum16::from_bits(0x3C00);
takum16 z = takum16::zero();
takum16 n = takum16::nar();         // Not-a-Real (analogous to NaN)

// Queries
a.is_zero();    // true
n.is_nar();     // true
a.bits();       // raw bit pattern as uint32_t

// Conversion
double  v = b.to_double();
int64_t i = b.to_int64();   // truncates; NaR → 0

// Arithmetic — NaR propagates; division by zero → NaR
takum16 sum  = b + c;
takum16 diff = b - c;
takum16 prod = b * c;
takum16 quot = b / c;
takum16 neg  = -b;

// Comparison — total order; NaR is the minimum element (smaller than all numbers)
b == c;  b != c;  b < c;  b <= c;  b > c;  b >= c;

// Output
std::cout << b;   // prints the double value, or "NaR" / "0"
```

## Building and Running Tests

```bash
cmake -B build
cmake --build build
ctest --test-dir build -V
```

Tests use [Catch2](https://github.com/catchorg/Catch2) (fetched automatically by CMake).

## Format Reference

### Linear Takum Encoding

Defined for bit-length `n >= 12`. Fields are laid out MSB→LSB:

| sign | direction | regime | characteristic | fraction |
|---|---|---|---|---|
| `S` | `D` | `R` | `C` | `F` |
| `1 bit` | `1 bit` | `3 bits` | `r bits` | `p bits` |

- `r = (D == 0) ? 7 - R : R` — regime value in [0, 7]
- `c = (D == 0) ? -2^(r+1) + 1 + C : 2^r - 1 + C` — characteristic
- `p = n - r - 5` — number of fraction bits
- `f = F * 2^(-p)` — fraction in [0, 1)
- `e = (-1)^S * (c + S)` — exponent in [-255, 254]

Special encodings (all non-sign bits zero):

| Bit pattern | Value | Notes |
|---|---|---|
| `S=0`, rest zero | 0 | bit pattern `0x00…0` |
| `S=1`, rest zero | NaR | bit pattern `0x80…0`; minimum of the total order |

The format admits a **total order** via signed integer comparison of the raw bit pattern: NaR sits at the minimum (most-negative signed integer for width N), enabling branchless sorting and min/max.

Representable range: `{0, NaR} ∪ [-2^255, -2^(-255)] ∪ [2^(-255), 2^255]`
