#include "SysVAbiHarness.h"

namespace {

using sysv_abi::Compiler;
using sysv_abi::linkRunExpect;

// INTEGER+INTEGER: __int128 in rdi+rsi / rax+rdx. Trans only copies; gcc forms the value.
constexpr const char* kInt128LibGcc = R"prg(
        __int128 make_i128(void) {
            return ((__int128)1 << 64) + 42;
        }
        long split_i128(__int128 x) {
            return (long)x + (long)(x >> 64);
        }
    )prg";

constexpr const char* kInt128MainTrans = R"prg(
        int printf(const char *, ...);
        __int128 make_i128(void);
        long split_i128(__int128);
        int main(void) {
            printf("%d", (int)split_i128(make_i128()));
            return 0;
        }
    )prg";

constexpr const char* kInt128LibTrans = R"prg(
        __int128 ident_i128(__int128 x) {
            return x;
        }
        long split_i128(__int128 x) {
            unsigned long *p;
            p = (unsigned long *)&x;
            return (long)(p[0] + p[1]);
        }
    )prg";

constexpr const char* kInt128MainGcc = R"prg(
        int printf(const char *, ...);
        __int128 ident_i128(__int128);
        long split_i128(__int128);
        int main(void) {
            __int128 v = ((__int128)1 << 64) + 42;
            __int128 r = ident_i128(v);
            printf("%d %d", (int)split_i128(v), (int)(long)r + (int)(long)(r >> 64));
            return 0;
        }
    )prg";

TEST(SysVAbi, int128Pass_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_i128_tg", Compiler::Gcc, Compiler::Trans,
            kInt128LibGcc, kInt128MainTrans, "43"));
}

TEST(SysVAbi, int128PassReturn_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_i128_gt", Compiler::Trans, Compiler::Gcc,
            kInt128LibTrans, kInt128MainGcc, "43 43"));
}

constexpr const char* kInt128WidenLibGcc = R"prg(
        int is_neg1(__int128 x) {
            return x == (__int128)-1;
        }
        int is_u64max(__int128 x) {
            return x == (__int128)(unsigned long)-1;
        }
        long low_word(__int128 x) {
            return (long)x;
        }
    )prg";

constexpr const char* kInt128WidenMainTrans = R"prg(
        int printf(const char *, ...);
        int is_neg1(__int128);
        int is_u64max(__int128);
        long low_word(__int128);
        int main(void) {
            long v;
            unsigned long u;
            v = -1;
            u = 0;
            u = u - 1;
            printf("%d %d %d", is_neg1(v), is_u64max(u), (int)low_word(v));
            return 0;
        }
    )prg";

constexpr const char* kInt128WidenLibTrans = R"prg(
        __int128 from_long(long v) {
            return v;
        }
        __int128 from_ulong(unsigned long u) {
            return u;
        }
        long to_long(__int128 x) {
            return x;
        }
    )prg";

constexpr const char* kInt128WidenMainGcc = R"prg(
        int printf(const char *, ...);
        __int128 from_long(long);
        __int128 from_ulong(unsigned long);
        long to_long(__int128);
        int main(void) {
            __int128 n = from_long(-1L);
            __int128 z = from_ulong((unsigned long)-1);
            __int128 w = ((__int128)1 << 64) + 42;
            printf("%d %d %d", (int)(n == (__int128)-1),
                    (int)(z == (__int128)(unsigned long)-1),
                    (int)to_long(w));
            return 0;
        }
    )prg";

TEST(SysVAbi, int128Widen_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_i128_widen_tg", Compiler::Gcc, Compiler::Trans,
            kInt128WidenLibGcc, kInt128WidenMainTrans, "1 1 -1"));
}

TEST(SysVAbi, int128Widen_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_i128_widen_gt", Compiler::Trans, Compiler::Gcc,
            kInt128WidenLibTrans, kInt128WidenMainGcc, "1 1 42"));
}

constexpr const char* kInt128ArithLibTrans = R"prg(
        __int128 add128(__int128 a, __int128 b) {
            return a + b;
        }
        __int128 sub128(__int128 a, __int128 b) {
            return a - b;
        }
        __int128 and128(__int128 a, __int128 b) {
            return a & b;
        }
        __int128 or128(__int128 a, __int128 b) {
            return a | b;
        }
        __int128 xor128(__int128 a, __int128 b) {
            return a ^ b;
        }
        __int128 not128(__int128 a) {
            return ~a;
        }
        __int128 neg128(__int128 a) {
            return -a;
        }
        int cmps128(__int128 a, __int128 b) {
            if (a < b) {
                return -1;
            }
            if (a > b) {
                return 1;
            }
            return 0;
        }
        int cmpu128(unsigned __int128 a, unsigned __int128 b) {
            if (a < b) {
                return -1;
            }
            if (a > b) {
                return 1;
            }
            return 0;
        }
    )prg";

constexpr const char* kInt128ArithMainGcc = R"prg(
        int printf(const char *, ...);
        __int128 add128(__int128, __int128);
        __int128 sub128(__int128, __int128);
        __int128 and128(__int128, __int128);
        __int128 or128(__int128, __int128);
        __int128 xor128(__int128, __int128);
        __int128 not128(__int128);
        __int128 neg128(__int128);
        int cmps128(__int128, __int128);
        int cmpu128(unsigned __int128, unsigned __int128);
        int main(void) {
            __int128 a = ((__int128)1 << 64) + 42;
            __int128 b = ((__int128)1 << 64) + 1;
            __int128 n = (__int128)-1;
            printf("%d %d %d %d %d %d %d %d %d",
                    (int)(add128(a, b) == a + b),
                    (int)(sub128(a, b) == a - b),
                    (int)(and128(a, b) == (a & b)),
                    (int)(or128(a, b) == (a | b)),
                    (int)(xor128(a, b) == (a ^ b)),
                    (int)(not128(a) == ~a),
                    (int)(neg128(a) == -a),
                    cmps128(n, a),
                    cmpu128((unsigned __int128)n, (unsigned __int128)a));
            return 0;
        }
    )prg";

TEST(SysVAbi, int128Arith_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_i128_arith_gt", Compiler::Trans, Compiler::Gcc,
            kInt128ArithLibTrans, kInt128ArithMainGcc, "1 1 1 1 1 1 1 -1 1"));
}

constexpr const char* kInt128ShiftLibTrans = R"prg(
        __int128 shl128(__int128 a, int n) {
            return a << n;
        }
        __int128 sar128(__int128 a, int n) {
            return a >> n;
        }
        unsigned __int128 shr128(unsigned __int128 a, int n) {
            return a >> n;
        }
    )prg";

constexpr const char* kInt128ShiftMainGcc = R"prg(
        int printf(const char *, ...);
        __int128 shl128(__int128, int);
        __int128 sar128(__int128, int);
        unsigned __int128 shr128(unsigned __int128, int);
        int main(void) {
            __int128 a = ((__int128)1 << 64) + 42;
            unsigned __int128 u = (unsigned __int128)-1;
            printf("%d %d %d %d %d",
                    (int)(shl128(a, 1) == a << 1),
                    (int)(shl128(a, 64) == a << 64),
                    (int)(sar128((__int128)-1, 1) == (__int128)-1),
                    (int)(sar128((__int128)-1, 64) == (__int128)-1),
                    (int)(shr128(u, 64) == (u >> 64)));
            return 0;
        }
    )prg";

TEST(SysVAbi, int128Shift_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_i128_shift_gt", Compiler::Trans, Compiler::Gcc,
            kInt128ShiftLibTrans, kInt128ShiftMainGcc, "1 1 1 1 1"));
}

constexpr const char* kInt128MulDivLibTrans = R"prg(
        __int128 mul128(__int128 a, __int128 b) {
            return a * b;
        }
        __int128 div128(__int128 a, __int128 b) {
            return a / b;
        }
        __int128 mod128(__int128 a, __int128 b) {
            return a % b;
        }
        unsigned __int128 udiv128(unsigned __int128 a, unsigned __int128 b) {
            return a / b;
        }
        unsigned __int128 umod128(unsigned __int128 a, unsigned __int128 b) {
            return a % b;
        }
    )prg";

constexpr const char* kInt128MulDivMainGcc = R"prg(
        int printf(const char *, ...);
        __int128 mul128(__int128, __int128);
        __int128 div128(__int128, __int128);
        __int128 mod128(__int128, __int128);
        unsigned __int128 udiv128(unsigned __int128, unsigned __int128);
        unsigned __int128 umod128(unsigned __int128, unsigned __int128);
        int main(void) {
            __int128 a = ((__int128)1 << 64) + 42;
            __int128 b = ((__int128)1 << 64) + 1;
            unsigned __int128 u = (unsigned __int128)-1;
            printf("%d %d %d %d %d",
                    (int)(mul128(a, 2) == a * 2),
                    (int)(div128(a, a) == 1),
                    (int)(mod128(a, b) == a % b),
                    (int)(udiv128(u, 2) == (u / 2)),
                    (int)(umod128(u, 2) == (u % 2)));
            return 0;
        }
    )prg";

TEST(SysVAbi, int128MulDiv_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_i128_muldiv_gt", Compiler::Trans, Compiler::Gcc,
            kInt128MulDivLibTrans, kInt128MulDivMainGcc, "1 1 1 1 1"));
}

constexpr const char* kInt128LiteralLibTrans = R"prg(
        __int128 lit128(void) {
            return 0x1000000000000002a;
        }
    )prg";

constexpr const char* kInt128LiteralMainGcc = R"prg(
        int printf(const char *, ...);
        __int128 lit128(void);
        int main(void) {
            __int128 want = ((__int128)1 << 64) + 42;
            printf("%d", (int)(lit128() == want));
            return 0;
        }
    )prg";

TEST(SysVAbi, int128Literal_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_i128_lit_gt", Compiler::Trans, Compiler::Gcc,
            kInt128LiteralLibTrans, kInt128LiteralMainGcc, "1"));
}

} // namespace
