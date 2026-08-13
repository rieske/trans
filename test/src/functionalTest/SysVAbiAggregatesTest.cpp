#include "SysVAbiInteropHarness.h"

namespace {

using sysv_abi_interop::bothDirections;
using sysv_abi_interop::bothDirectionsLibs;

// INTEGER+INTEGER: two longs in rdi+rsi / rax+rdx.
constexpr const char* kPairLib = R"prg(
        struct Pair {
            long a;
            long b;
        };
        long sum_pair(struct Pair p) {
            return p.a + p.b;
        }
    )prg";

constexpr const char* kPairMain = R"prg(
        int printf(const char *, ...);
        struct Pair {
            long a;
            long b;
        };
        long sum_pair(struct Pair p);
        struct Pair arg;
        int main(void) {
            arg.a = 3;
            arg.b = 4;
            printf("%d", (int)sum_pair(arg));
            return 0;
        }
    )prg";

constexpr const char* kPairReturnLib = R"prg(
        struct Pair {
            long a;
            long b;
        };
        struct Pair make_pair(void) {
            struct Pair p;
            p.a = 3;
            p.b = 4;
            return p;
        }
    )prg";

constexpr const char* kPairReturnMain = R"prg(
        int printf(const char *, ...);
        struct Pair {
            long a;
            long b;
        };
        struct Pair make_pair(void);
        int main(void) {
            struct Pair p;
            p = make_pair();
            printf("%d %d", (int)p.a, (int)p.b);
            return 0;
        }
    )prg";

// SSE: one double in xmm0.
constexpr const char* kDoubleLib = R"prg(
        struct D {
            double d;
        };
        int as_int(struct D s) {
            return (int)s.d;
        }
    )prg";

constexpr const char* kDoubleMain = R"prg(
        int printf(const char *, ...);
        struct D {
            double d;
        };
        int as_int(struct D s);
        struct D arg;
        int main(void) {
            arg.d = 42.9;
            printf("%d", as_int(arg));
            return 0;
        }
    )prg";

constexpr const char* kDoubleReturnLib = R"prg(
        struct D {
            double d;
        };
        struct D make_d(void) {
            struct D s;
            s.d = 42.9;
            return s;
        }
    )prg";

constexpr const char* kDoubleReturnMain = R"prg(
        int printf(const char *, ...);
        struct D {
            double d;
        };
        struct D make_d(void);
        int main(void) {
            struct D s;
            s = make_d();
            printf("%d", (int)s.d);
            return 0;
        }
    )prg";

// INTEGER+SSE: int in rdi (or rax), double in xmm0.
constexpr const char* kMixedLib = R"prg(
        struct Mix {
            int x;
            double y;
        };
        int sum_mix(struct Mix s) {
            return s.x + (int)s.y;
        }
    )prg";

constexpr const char* kMixedMain = R"prg(
        int printf(const char *, ...);
        struct Mix {
            int x;
            double y;
        };
        int sum_mix(struct Mix s);
        struct Mix arg;
        int main(void) {
            arg.x = 3;
            arg.y = 4.8;
            printf("%d", sum_mix(arg));
            return 0;
        }
    )prg";

constexpr const char* kMixedReturnLib = R"prg(
        struct Mix {
            int x;
            double y;
        };
        struct Mix make_mix(void) {
            struct Mix s;
            s.x = 3;
            s.y = 4.8;
            return s;
        }
    )prg";

constexpr const char* kMixedReturnMain = R"prg(
        int printf(const char *, ...);
        struct Mix {
            int x;
            double y;
        };
        struct Mix make_mix(void);
        int main(void) {
            struct Mix s;
            s = make_mix();
            printf("%d %d", s.x, (int)s.y);
            return 0;
        }
    )prg";

// SSE: two packed floats in xmm0.
constexpr const char* kTwoFloatLib = R"prg(
        struct FF {
            float a;
            float b;
        };
        int sum_ff(struct FF s) {
            return (int)s.a + (int)s.b;
        }
    )prg";

constexpr const char* kTwoFloatMain = R"prg(
        int printf(const char *, ...);
        struct FF {
            float a;
            float b;
        };
        int sum_ff(struct FF s);
        struct FF arg;
        int main(void) {
            arg.a = 20.0f;
            arg.b = 22.0f;
            printf("%d", sum_ff(arg));
            return 0;
        }
    )prg";

constexpr const char* kVaArgPairMain = R"prg(
        int printf(const char *, ...);
        struct Pair {
            long a;
            long b;
        };
        long sum_extra(int n, ...);
        struct Pair arg;
        int main(void) {
            arg.a = 11;
            arg.b = 22;
            printf("%d", (int)sum_extra(0, arg));
            return 0;
        }
    )prg";

constexpr const char* kVaArgPairLibTrans = R"prg(
        struct Pair {
            long a;
            long b;
        };
        long sum_extra(int n, ...) {
            __builtin_va_list ap;
            struct Pair p;
            __builtin_va_start(ap, n);
            p = __builtin_va_arg(ap, struct Pair);
            __builtin_va_end(ap);
            return p.a + p.b;
        }
    )prg";

constexpr const char* kVaArgPairLibGcc = R"prg(
        #include <stdarg.h>
        struct Pair {
            long a;
            long b;
        };
        long sum_extra(int n, ...) {
            va_list ap;
            struct Pair p;
            va_start(ap, n);
            p = va_arg(ap, struct Pair);
            va_end(ap);
            return p.a + p.b;
        }
    )prg";

// MEMORY return of a variadic function still uses sret (hidden rdi).
constexpr const char* kVariadicSretLib = R"prg(
        struct Big {
            long a;
            long b;
            long c;
        };
        struct Big make_big(int n, ...) {
            struct Big s;
            s.a = 1;
            s.b = 2;
            s.c = 3;
            (void)n;
            return s;
        }
    )prg";

constexpr const char* kVariadicSretMain = R"prg(
        int printf(const char *, ...);
        struct Big {
            long a;
            long b;
            long c;
        };
        struct Big make_big(int n, ...);
        int main(void) {
            struct Big s;
            s = make_big(0, 0);
            printf("%d %d %d", (int)s.a, (int)s.b, (int)s.c);
            return 0;
        }
    )prg";

TEST(SysVAbi, pairPass) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_pair_pass", kPairLib, kPairMain, "7"));
}

TEST(SysVAbi, pairReturn) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_pair_ret", kPairReturnLib, kPairReturnMain, "3 4"));
}

TEST(SysVAbi, doubleStructPass) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_d_pass", kDoubleLib, kDoubleMain, "42"));
}

TEST(SysVAbi, doubleStructReturn) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_d_ret", kDoubleReturnLib, kDoubleReturnMain, "42"));
}

TEST(SysVAbi, mixedIntDoublePass) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_mix_pass", kMixedLib, kMixedMain, "7"));
}

TEST(SysVAbi, mixedIntDoubleReturn) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_mix_ret", kMixedReturnLib, kMixedReturnMain, "3 4"));
}

TEST(SysVAbi, twoFloatStructPass) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_ff_pass", kTwoFloatLib, kTwoFloatMain, "42"));
}

TEST(SysVAbi, vaArgTwoWordStruct) {
    ASSERT_NO_FATAL_FAILURE(bothDirectionsLibs(
            "sysv_va_pair", kVaArgPairLibTrans, kVaArgPairLibGcc, kVaArgPairMain, "33"));
}

TEST(SysVAbi, variadicMemoryReturn) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_va_sret", kVariadicSretLib, kVariadicSretMain, "1 2 3"));
}


// SSE+INTEGER: double in xmm0, long in rax (not rdx). Eightbyte 1 is the first INTEGER.
constexpr const char* kSseThenIntegerReturnLib = R"prg(
        struct DL {
            double d;
            long l;
        };
        struct DL make_dl(void) {
            struct DL s;
            s.d = 42.0;
            s.l = 99;
            return s;
        }
    )prg";

constexpr const char* kSseThenIntegerReturnMain = R"prg(
        int printf(const char *, ...);
        struct DL {
            double d;
            long l;
        };
        struct DL make_dl(void);
        int main(void) {
            struct DL s;
            s = make_dl();
            printf("%d %d", (int)s.d, (int)s.l);
            return 0;
        }
    )prg";

TEST(SysVAbi, sseThenIntegerReturn) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_sse_int_ret", kSseThenIntegerReturnLib, kSseThenIntegerReturnMain, "42 99"));
}
} // namespace
