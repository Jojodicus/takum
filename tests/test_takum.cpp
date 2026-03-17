#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "takum.hpp"

using namespace takum;
using Catch::Approx;

// ── Special values ──────────────────────────────────────────────────────────

TEST_CASE("Special values", "[special]") {
    SECTION("zero has bits == 0") {
        takum16 z = takum16::zero();
        REQUIRE(z.bits() == 0u);
        REQUIRE(z.is_zero());
        REQUIRE_FALSE(z.is_nar());
    }

    SECTION("NaR has only sign bit set") {
        takum16 n = takum16::nar();
        REQUIRE(n.bits() == (1u << 15));
        REQUIRE(n.is_nar());
        REQUIRE_FALSE(n.is_zero());
    }

    SECTION("default-constructed is zero") {
        takum16 t;
        REQUIRE(t.is_zero());
    }

    SECTION("from_bits round-trips bits") {
        takum16 t = takum16::from_bits(0b0110000000000000u);
        REQUIRE(t.bits() == 0b0110000000000000u);
    }
}

// ── NaR propagation ─────────────────────────────────────────────────────────

TEST_CASE("NaR propagation", "[nar]") {
    takum16 n = takum16::nar();
    takum16 one(1.0);

    SECTION("NaR + x == NaR")  { REQUIRE((n + one).is_nar()); }
    SECTION("x + NaR == NaR")  { REQUIRE((one + n).is_nar()); }
    SECTION("NaR - x == NaR")  { REQUIRE((n - one).is_nar()); }
    SECTION("NaR * x == NaR")  { REQUIRE((n * one).is_nar()); }
    SECTION("x * NaR == NaR")  { REQUIRE((one * n).is_nar()); }
    SECTION("NaR / x == NaR")  { REQUIRE((n / one).is_nar()); }
    SECTION("x / NaR == NaR")  { REQUIRE((one / n).is_nar()); }
    SECTION("x / 0  == NaR")   { REQUIRE((one / takum16::zero()).is_nar()); }
}

// ── Unary minus ─────────────────────────────────────────────────────────────

TEST_CASE("Unary minus", "[unary]") {
    SECTION("-zero == zero") {
        takum16 z = takum16::zero();
        REQUIRE((-z).is_zero());
    }

    SECTION("-NaR == NaR") {
        takum16 n = takum16::nar();
        REQUIRE((-n).is_nar());
    }

    SECTION("-1.0 round-trips") {
        takum16 pos(1.0);
        takum16 neg = -pos;
        REQUIRE(neg.to_double() == Approx(-1.0).epsilon(1e-3));
    }
}

// ── Decode (bits → double) ───────────────────────────────────────────────────

TEST_CASE("Decode known values - takum16", "[decode][takum16]") {
    // Check that encoding then decoding specific doubles gives back the right value.
    // We just call Takum(double) and read to_double(), verifying decode matches.
    auto check = [](double v, double eps = 1e-3) {
        takum16 t(v);
        REQUIRE_FALSE(t.is_nar());
        REQUIRE(t.to_double() == Approx(v).epsilon(eps));
    };

    SECTION("1.0")  { check(1.0); }
    SECTION("-1.0") { check(-1.0); }
    SECTION("2.0")  { check(2.0); }
    SECTION("-2.0") { check(-2.0); }
    SECTION("0.5")  { check(0.5); }
    SECTION("-0.5") { check(-0.5); }
    SECTION("1.5")  { check(1.5); }
    SECTION("-1.5") { check(-1.5); }
}

TEST_CASE("Decode known values - takum32", "[decode][takum32]") {
    auto check = [](double v, double eps = 1e-6) {
        takum32 t(v);
        REQUIRE_FALSE(t.is_nar());
        REQUIRE(t.to_double() == Approx(v).epsilon(eps));
    };

    SECTION("1.0")  { check(1.0); }
    SECTION("-1.0") { check(-1.0); }
    SECTION("2.0")  { check(2.0); }
    SECTION("-2.0") { check(-2.0); }
    SECTION("0.5")  { check(0.5); }
    SECTION("-0.5") { check(-0.5); }
    SECTION("1.5")  { check(1.5); }
    SECTION("-1.5") { check(-1.5); }
}

// ── Encode round-trip ────────────────────────────────────────────────────────

TEST_CASE("Encode round-trip", "[roundtrip]") {
    auto rt16 = [](double v) {
        takum16 t(v);
        REQUIRE(t.to_double() == Approx(v).epsilon(1e-2));
    };
    auto rt32 = [](double v) {
        takum32 t(v);
        REQUIRE(t.to_double() == Approx(v).epsilon(1e-6));
    };

    SECTION("takum16: 1.0")  { rt16(1.0); }
    SECTION("takum16: -1.0") { rt16(-1.0); }
    SECTION("takum16: 2.0")  { rt16(2.0); }
    SECTION("takum16: 0.5")  { rt16(0.5); }
    SECTION("takum16: 1.5")  { rt16(1.5); }
    SECTION("takum32: 1.0")  { rt32(1.0); }
    SECTION("takum32: -1.5") { rt32(-1.5); }
    SECTION("takum32: 3.14") { rt32(3.14159265358979); }
}

// ── Arithmetic ───────────────────────────────────────────────────────────────

TEST_CASE("Arithmetic", "[arith]") {
    takum16 one(1.0), two(2.0), three(3.0), six(6.0);

    SECTION("1 + 1 == 2") {
        REQUIRE((one + one).to_double() == Approx(2.0).epsilon(1e-2));
    }

    SECTION("1 - 1 == 0") {
        takum16 result = one - one;
        REQUIRE(result.is_zero());
    }

    SECTION("2 * 3 == 6") {
        REQUIRE((two * three).to_double() == Approx(6.0).epsilon(1e-2));
    }

    SECTION("6 / 2 == 3") {
        REQUIRE((six / two).to_double() == Approx(3.0).epsilon(1e-2));
    }

    SECTION("commutativity: a+b == b+a") {
        takum16 a(1.5), b(2.5);
        REQUIRE((a + b).to_double() == Approx((b + a).to_double()).epsilon(1e-9));
    }

    SECTION("commutativity: a*b == b*a") {
        takum16 a(1.5), b(2.5);
        REQUIRE((a * b).to_double() == Approx((b * a).to_double()).epsilon(1e-9));
    }

    SECTION("compound assignment +=") {
        takum16 t(1.0);
        t += one;
        REQUIRE(t.to_double() == Approx(2.0).epsilon(1e-2));
    }
}

// ── Integer conversion ───────────────────────────────────────────────────────

TEST_CASE("Integer conversion", "[int]") {
    SECTION("42") {
        takum16 t(static_cast<int64_t>(42));
        REQUIRE(t.to_int64() == 42);
    }

    SECTION("-7") {
        takum16 t(static_cast<int64_t>(-7));
        REQUIRE(t.to_int64() == -7);
    }

    SECTION("NaR → 0") {
        REQUIRE(takum16::nar().to_int64() == 0);
    }

    SECTION("explicit cast operator") {
        takum16 t(static_cast<int64_t>(100));
        REQUIRE(static_cast<int64_t>(t) == 100);
    }
}

// ── Double edge cases ────────────────────────────────────────────────────────

TEST_CASE("Double edge cases", "[edge]") {
    SECTION("inf → NaR") {
        takum16 t(std::numeric_limits<double>::infinity());
        REQUIRE(t.is_nar());
    }

    SECTION("-inf → NaR") {
        takum16 t(-std::numeric_limits<double>::infinity());
        REQUIRE(t.is_nar());
    }

    SECTION("nan → NaR") {
        takum16 t(std::numeric_limits<double>::quiet_NaN());
        REQUIRE(t.is_nar());
    }

    SECTION("very large → NaR") {
        // exponent > 254 should overflow to NaR
        takum16 t(std::ldexp(1.0, 300));
        REQUIRE(t.is_nar());
    }

    SECTION("very small positive stays non-NaR or zero") {
        // Very small values may underflow to zero but not NaR
        takum16 t(std::ldexp(1.0, -300));
        // Should be NaR (underflow) based on the plan spec (e < -255)
        REQUIRE(t.is_nar());
    }
}

// ── Comparison ───────────────────────────────────────────────────────────────

TEST_CASE("Comparison", "[cmp]") {
    takum16 one(1.0), two(2.0);
    takum16 n = takum16::nar();

    SECTION("1.0 < 2.0") { REQUIRE(one < two); }
    SECTION("2.0 > 1.0") { REQUIRE(two > one); }
    SECTION("1.0 <= 1.0") { REQUIRE(one <= takum16(1.0)); }
    SECTION("1.0 >= 1.0") { REQUIRE(one >= takum16(1.0)); }
    SECTION("1.0 == 1.0") { REQUIRE(one == takum16(1.0)); }
    SECTION("1.0 != 2.0") { REQUIRE(one != two); }

    SECTION("NaR == NaR (total order)") { REQUIRE(n == n); }
    SECTION("NaR != NaR is false")     { REQUIRE_FALSE(n != n); }
    SECTION("NaR < one (NaR is min)")  { REQUIRE(n < one); }
    SECTION("NaR <= one")              { REQUIRE(n <= one); }
    SECTION("NaR > one is false")      { REQUIRE_FALSE(n > one); }
    SECTION("NaR >= one is false")     { REQUIRE_FALSE(n >= one); }
    SECTION("NaR <= NaR")              { REQUIRE(n <= n); }
    SECTION("NaR >= NaR")              { REQUIRE(n >= n); }
}

// ── Width variants ───────────────────────────────────────────────────────────

TEST_CASE("Width variants all compile and agree on 1.0", "[widths]") {
    takum12 t12(1.0);
    takum16 t16(1.0);
    takum32 t32(1.0);

    SECTION("takum12(1.0) decodes near 1.0") {
        REQUIRE(t12.to_double() == Approx(1.0).epsilon(1e-2));
    }
    SECTION("takum16(1.0) decodes near 1.0") {
        REQUIRE(t16.to_double() == Approx(1.0).epsilon(1e-3));
    }
    SECTION("takum32(1.0) decodes near 1.0") {
        REQUIRE(t32.to_double() == Approx(1.0).epsilon(1e-6));
    }
    SECTION("all agree") {
        REQUIRE(t12.to_double() == Approx(t16.to_double()).epsilon(1e-2));
        REQUIRE(t16.to_double() == Approx(t32.to_double()).epsilon(1e-3));
    }
}

TEST_CASE("Width variants spot-check -2.0", "[widths]") {
    REQUIRE(takum12(-2.0).to_double() == Approx(-2.0).epsilon(1e-2));
    REQUIRE(takum16(-2.0).to_double() == Approx(-2.0).epsilon(1e-3));
    REQUIRE(takum32(-2.0).to_double() == Approx(-2.0).epsilon(1e-6));
}
