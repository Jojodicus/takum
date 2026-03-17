#pragma once

#include <cstdint>
#include <cmath>
#include <limits>
#include <ostream>

namespace takum {

template<unsigned N>  // 12 <= N <= 48
class Takum {
    static_assert(N >= 12 && N <= 48, "Takum width N must be in [12, 48]");

public:
    Takum() noexcept : bits_(0) {}

    explicit Takum(double v) : bits_(encode(v)) {}

    explicit Takum(int64_t v) : bits_(encode(static_cast<double>(v))) {}

    static Takum from_bits(uint64_t b) noexcept {
        Takum t;
        t.bits_ = b & mask();
        return t;
    }

    static Takum zero() noexcept { return Takum(); }

    static Takum nar() noexcept {
        Takum t;
        t.bits_ = uint64_t(1) << (N - 1);
        return t;
    }

    bool is_zero() const noexcept { return bits_ == 0; }

    bool is_nar() const noexcept { return bits_ == (uint64_t(1) << (N - 1)); }

    uint64_t bits() const noexcept { return bits_; }

    double to_double() const noexcept { return decode(bits_); }

    int64_t to_int64() const noexcept {
        double d = to_double();
        if (std::isnan(d)) return 0;
        return static_cast<int64_t>(d);
    }

    explicit operator double()  const noexcept { return to_double(); }
    explicit operator int64_t() const noexcept { return to_int64(); }

    Takum operator+(const Takum& o) const {
        if (is_nar() || o.is_nar()) return nar();
        return Takum(to_double() + o.to_double());
    }

    Takum operator-(const Takum& o) const {
        if (is_nar() || o.is_nar()) return nar();
        return Takum(to_double() - o.to_double());
    }

    Takum operator*(const Takum& o) const {
        if (is_nar() || o.is_nar()) return nar();
        return Takum(to_double() * o.to_double());
    }

    Takum operator/(const Takum& o) const {
        if (is_nar() || o.is_nar()) return nar();
        if (o.is_zero()) return nar();
        return Takum(to_double() / o.to_double());
    }

    Takum operator-() const noexcept {
        if (is_zero()) return zero();
        if (is_nar())  return nar();
        return Takum(-to_double());
    }

    Takum& operator+=(const Takum& o) { *this = *this + o; return *this; }
    Takum& operator-=(const Takum& o) { *this = *this - o; return *this; }
    Takum& operator*=(const Takum& o) { *this = *this * o; return *this; }
    Takum& operator/=(const Takum& o) { *this = *this / o; return *this; }

    // Comparisons use integer ordering on the sign-extended bit pattern.
    // NaR (= 1<<(N-1), the minimum signed N-bit integer) is the smallest element.
    bool operator==(const Takum& o) const noexcept { return bits_ == o.bits_; }
    bool operator!=(const Takum& o) const noexcept { return bits_ != o.bits_; }
    bool operator< (const Takum& o) const noexcept { return signed_bits() <  o.signed_bits(); }
    bool operator<=(const Takum& o) const noexcept { return signed_bits() <= o.signed_bits(); }
    bool operator> (const Takum& o) const noexcept { return signed_bits() >  o.signed_bits(); }
    bool operator>=(const Takum& o) const noexcept { return signed_bits() >= o.signed_bits(); }

    friend std::ostream& operator<<(std::ostream& os, const Takum& t) {
        if (t.is_nar())  return os << "NaR";
        if (t.is_zero()) return os << "0";
        return os << t.to_double();
    }

private:
    uint64_t bits_;

    // Sign-extend the N-bit pattern to int64_t for total-order comparison.
    int64_t signed_bits() const noexcept {
        if (bits_ & (uint64_t(1) << (N - 1)))
            return static_cast<int64_t>(bits_ | ~mask());
        return static_cast<int64_t>(bits_);
    }

    static constexpr uint64_t mask() {
        return (N == 64) ? ~uint64_t(0) : ((uint64_t(1) << N) - 1u);
    }

    static int ilog2(unsigned x) {
        if (x == 0) return -1;
        return 31 - __builtin_clz(x);
    }

    static double decode(uint64_t bits) noexcept {
        const uint64_t sign_bit = uint64_t(1) << (N - 1);

        unsigned S = (bits >> (N - 1)) & 1u;

        // zero or NaR: lower (N-1) bits all zero
        if ((bits & (sign_bit - 1u)) == 0) {
            return S ? std::numeric_limits<double>::quiet_NaN() : 0.0;
        }

        unsigned D = (bits >> (N - 2)) & 1u;
        unsigned R = (bits >> (N - 5)) & 0x7u;  // 3-bit field
        unsigned r = (D == 0) ? (7u - R) : R;    // actual regime size in [0,7]
        int      p = static_cast<int>(N) - static_cast<int>(r) - 5; // fraction bits

        uint64_t C_mask = (r > 0) ? ((uint64_t(1) << r) - 1u) : 0u;
        uint64_t F_mask = (p > 0) ? ((uint64_t(1) << p) - 1u) : 0u;

        uint64_t C_field = (bits >> p) & C_mask;
        uint64_t F_field =  bits       & F_mask;

        int c;
        if (D == 0) {
            c = -(1 << (r + 1)) + 1 + static_cast<int>(C_field);
        } else {
            c = (1 << r) - 1 + static_cast<int>(C_field);
        }

        int e = (S == 0) ? c : -(c + 1);

        double f = (p > 0) ? (static_cast<double>(F_field) * std::ldexp(1.0, -p)) : 0.0;

        double mantissa = (S == 0) ? (1.0 + f) : (-2.0 + f);
        return mantissa * std::ldexp(1.0, e);
    }

    static uint64_t encode(double v) noexcept {
        const uint64_t nar_bits = uint64_t(1) << (N - 1);

        // Special cases
        if (v == 0.0) return 0u;
        if (std::isnan(v) || std::isinf(v)) return nar_bits;

        unsigned S;
        int e;
        double f;
        int c;

        if (v > 0.0) {
            S = 0;
            int k;
            double m = std::frexp(v, &k);
            e = k - 1;
            f = 2.0 * m - 1.0;  // in [0, 1)
            c = e;
        } else {
            S = 1;
            double av = -v;
            int k;
            double m = std::frexp(av, &k);
            if (m == 0.5) {
                // exact power of 2
                e = k - 2;
                f = 0.0;
            } else {
                e = k - 1;
                f = 2.0 * (1.0 - m);  // in (0, 1)
            }
            c = -(e + 1);
        }

        // Overflow / underflow → NaR
        if (e < -255 || e > 254) return nar_bits;

        // Find D, r, R, C_field from c
        unsigned D, r, R, C_field;
        if (c < 0) {
            D = 0;
            unsigned neg_c = static_cast<unsigned>(-c);
            r = static_cast<unsigned>(ilog2(neg_c));
            R = 7u - r;
            C_field = static_cast<unsigned>(c + (1 << (r + 1)) - 1);
        } else {
            D = 1;
            r = (c == 0) ? 0u : static_cast<unsigned>(ilog2(static_cast<unsigned>(c + 1)));
            R = r;
            C_field = static_cast<unsigned>(c) - ((1u << r) - 1u);
        }

        int p = static_cast<int>(N) - static_cast<int>(r) - 5;
        uint64_t F_field = 0u;
        if (p > 0) {
            F_field = static_cast<uint64_t>(f * std::ldexp(1.0, p));
            uint64_t p_max = uint64_t(1) << p;
            if (F_field >= p_max) F_field = p_max - 1u;
        }

        uint64_t bits = (uint64_t(S) << (N - 1)) | (uint64_t(D) << (N - 2))
                      | (uint64_t(R) << (N - 5)) | (uint64_t(C_field) << p)
                      | F_field;
        bits &= mask();
        return bits;
    }
};

using takum12 = Takum<12>;
using takum16 = Takum<16>;
using takum32 = Takum<32>;
using takum48 = Takum<48>;

} // namespace takum
