#include "SysVAbiHarness.h"

namespace {

using sysv_abi::Compiler;
using sysv_abi::linkRunExpect;

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

TEST(SysVAbi, pairPass_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_pair_pass_tg", Compiler::Gcc, Compiler::Trans, kPairLib, kPairMain, "7"));
}

TEST(SysVAbi, pairPass_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_pair_pass_gt", Compiler::Trans, Compiler::Gcc, kPairLib, kPairMain, "7"));
}

TEST(SysVAbi, pairReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_pair_ret_tg", Compiler::Gcc, Compiler::Trans, kPairReturnLib, kPairReturnMain, "3 4"));
}

TEST(SysVAbi, pairReturn_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_pair_ret_gt", Compiler::Trans, Compiler::Gcc, kPairReturnLib, kPairReturnMain, "3 4"));
}

TEST(SysVAbi, doubleStructPass_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_d_pass_tg", Compiler::Gcc, Compiler::Trans, kDoubleLib, kDoubleMain, "42"));
}

TEST(SysVAbi, doubleStructPass_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_d_pass_gt", Compiler::Trans, Compiler::Gcc, kDoubleLib, kDoubleMain, "42"));
}

TEST(SysVAbi, doubleStructReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_d_ret_tg", Compiler::Gcc, Compiler::Trans, kDoubleReturnLib, kDoubleReturnMain, "42"));
}

TEST(SysVAbi, doubleStructReturn_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_d_ret_gt", Compiler::Trans, Compiler::Gcc, kDoubleReturnLib, kDoubleReturnMain, "42"));
}

TEST(SysVAbi, mixedIntDoublePass_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_mix_pass_tg", Compiler::Gcc, Compiler::Trans, kMixedLib, kMixedMain, "7"));
}

TEST(SysVAbi, mixedIntDoublePass_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_mix_pass_gt", Compiler::Trans, Compiler::Gcc, kMixedLib, kMixedMain, "7"));
}

TEST(SysVAbi, mixedIntDoubleReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_mix_ret_tg", Compiler::Gcc, Compiler::Trans, kMixedReturnLib, kMixedReturnMain, "3 4"));
}

TEST(SysVAbi, mixedIntDoubleReturn_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_mix_ret_gt", Compiler::Trans, Compiler::Gcc, kMixedReturnLib, kMixedReturnMain, "3 4"));
}

TEST(SysVAbi, twoFloatStructPass_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_ff_pass_tg", Compiler::Gcc, Compiler::Trans, kTwoFloatLib, kTwoFloatMain, "42"));
}

TEST(SysVAbi, twoFloatStructPass_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_ff_pass_gt", Compiler::Trans, Compiler::Gcc, kTwoFloatLib, kTwoFloatMain, "42"));
}

TEST(SysVAbi, vaArgTwoWordStruct_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_va_pair_tg", Compiler::Gcc, Compiler::Trans, kVaArgPairLibGcc, kVaArgPairMain, "33"));
}

TEST(SysVAbi, vaArgTwoWordStruct_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_va_pair_gt", Compiler::Trans, Compiler::Gcc, kVaArgPairLibTrans, kVaArgPairMain, "33"));
}

TEST(SysVAbi, variadicMemoryReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_va_sret_tg", Compiler::Gcc, Compiler::Trans, kVariadicSretLib, kVariadicSretMain, "1 2 3"));
}

TEST(SysVAbi, variadicMemoryReturn_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_va_sret_gt", Compiler::Trans, Compiler::Gcc, kVariadicSretLib, kVariadicSretMain, "1 2 3"));
}

constexpr const char* kLongDoubleReturnLib = R"prg(
        long double make_ld(void) {
            return 42.5L;
        }
    )prg";

constexpr const char* kLongDoubleReturnMain = R"prg(
        int printf(const char *, ...);
        long double make_ld(void);
        int main(void) {
            long double x;
            x = make_ld();
            printf("%.1Lf", x);
            return 0;
        }
    )prg";

constexpr const char* kLongDoubleIdentLib = R"prg(
        long double ident_ld(long double x) {
            return x;
        }
    )prg";

constexpr const char* kLongDoubleIdentMain = R"prg(
        int printf(const char *, ...);
        long double ident_ld(long double);
        long double arg;
        int main(void) {
            arg = 42.5L;
            printf("%.1Lf", ident_ld(arg));
            return 0;
        }
    )prg";

constexpr const char* kTwoLongDoubleLib = R"prg(
        long double make_a(void) {
            return 20.0L;
        }
        long double make_b(void) {
            return 22.0L;
        }
        int sum_ld(long double a, long double b) {
            return (int)(a + b);
        }
    )prg";

constexpr const char* kTwoLongDoubleMain = R"prg(
        int printf(const char *, ...);
        long double make_a(void);
        long double make_b(void);
        int sum_ld(long double, long double);
        int main(void) {
            printf("%d", sum_ld(make_a(), make_b()));
            return 0;
        }
    )prg";

constexpr const char* kSecondLongDoubleLib = R"prg(
        long double second_ld(long double a, long double b) {
            return b;
        }
    )prg";

constexpr const char* kSecondLongDoubleMain = R"prg(
        int printf(const char *, ...);
        long double second_ld(long double, long double);
        long double a;
        long double b;
        int main(void) {
            a = 1.0L;
            b = 42.0L;
            printf("%.0Lf", second_ld(a, b));
            return 0;
        }
    )prg";

constexpr const char* kVaArgLongDoubleLibTrans = R"prg(
        long double take_ld(int a, int b, int c, int d, int e, int f, ...) {
            __builtin_va_list ap;
            int n;
            long double x;
            __builtin_va_start(ap, f);
            n = __builtin_va_arg(ap, int);
            x = __builtin_va_arg(ap, long double);
            __builtin_va_end(ap);
            (void)n;
            return x;
        }
    )prg";

constexpr const char* kVaArgLongDoubleLibGcc = R"prg(
        #include <stdarg.h>
        long double make_ld(void) {
            return 42.0L;
        }
        int take_ld(int a, int b, int c, int d, int e, int f, ...) {
            va_list ap;
            int n;
            long double x;
            va_start(ap, f);
            n = va_arg(ap, int);
            x = va_arg(ap, long double);
            va_end(ap);
            (void)n;
            return (int)x;
        }
    )prg";

constexpr const char* kVaArgLongDoubleMainTrans = R"prg(
        int printf(const char *, ...);
        long double take_ld(int, int, int, int, int, int, ...);
        int main(void) {
            printf("%.0Lf", take_ld(1, 2, 3, 4, 5, 6, 7, 42.0L));
            return 0;
        }
    )prg";

constexpr const char* kVaArgLongDoubleMainGcc = R"prg(
        int printf(const char *, ...);
        long double make_ld(void);
        int take_ld(int, int, int, int, int, int, ...);
        int main(void) {
            printf("%d", take_ld(1, 2, 3, 4, 5, 6, 7, make_ld()));
            return 0;
        }
    )prg";

constexpr const char* kLongDoubleLiteralLib = R"prg(
        long double make_ld(void) {
            return 42.5L;
        }
    )prg";

constexpr const char* kLongDoubleLiteralMain = R"prg(
        int printf(const char *, ...);
        long double make_ld(void);
        int main(void) {
            printf("%.1Lf", make_ld());
            return 0;
        }
    )prg";

constexpr const char* kLongDoubleAddLib = R"prg(
        long double add_ld(long double a, long double b) {
            return a + b;
        }
    )prg";

constexpr const char* kLongDoubleAddMain = R"prg(
        int printf(const char *, ...);
        long double add_ld(long double, long double);
        int main(void) {
            printf("%.1Lf", add_ld(20.0L, 22.5L));
            return 0;
        }
    )prg";

TEST(SysVAbi, longDoubleAdd_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_ld_add_gt", Compiler::Trans, Compiler::Gcc,
            kLongDoubleAddLib, kLongDoubleAddMain, "42.5"));
}

TEST(SysVAbi, longDoubleLiteral_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_ld_lit_gt", Compiler::Trans, Compiler::Gcc,
            kLongDoubleLiteralLib, kLongDoubleLiteralMain, "42.5"));
}

TEST(SysVAbi, longDoubleReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_ld_ret_tg", Compiler::Gcc, Compiler::Trans,
            kLongDoubleReturnLib, kLongDoubleReturnMain, "42.5"));
}

TEST(SysVAbi, longDoubleReturn_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_ld_ret_gt", Compiler::Trans, Compiler::Gcc,
            kLongDoubleIdentLib, kLongDoubleIdentMain, "42.5"));
}

TEST(SysVAbi, twoLongDoublePass_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_ld2_pass_tg", Compiler::Gcc, Compiler::Trans,
            kTwoLongDoubleLib, kTwoLongDoubleMain, "42"));
}

TEST(SysVAbi, twoLongDoublePass_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_ld2_pass_gt", Compiler::Trans, Compiler::Gcc,
            kSecondLongDoubleLib, kSecondLongDoubleMain, "42"));
}

constexpr const char* kStackIntThenLdLibGcc = R"prg(
        long double make_ld(void) {
            return 42.0L;
        }
        int after_int(int a, int b, int c, int d, int e, int f, int g, long double ld) {
            return (int)ld + g;
        }
    )prg";

constexpr const char* kStackIntThenLdMainTrans = R"prg(
        int printf(const char *, ...);
        long double make_ld(void);
        int after_int(int, int, int, int, int, int, int, long double);
        int main(void) {
            printf("%d", after_int(1, 2, 3, 4, 5, 6, 7, make_ld()));
            return 0;
        }
    )prg";

constexpr const char* kStackIntThenLdLibTrans = R"prg(
        long double after_int(int a, int b, int c, int d, int e, int f, int g, long double ld) {
            (void)a;
            (void)b;
            (void)c;
            (void)d;
            (void)e;
            (void)f;
            (void)g;
            return ld;
        }
    )prg";

constexpr const char* kStackIntThenLdMainGcc = R"prg(
        int printf(const char *, ...);
        long double after_int(int, int, int, int, int, int, int, long double);
        int main(void) {
            printf("%.0Lf", after_int(1, 2, 3, 4, 5, 6, 7, 42.0L));
            return 0;
        }
    )prg";

TEST(SysVAbi, stackIntThenLongDouble_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_stack_i_ld_tg", Compiler::Gcc, Compiler::Trans,
            kStackIntThenLdLibGcc, kStackIntThenLdMainTrans, "49"));
}

TEST(SysVAbi, stackIntThenLongDouble_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_stack_i_ld_gt", Compiler::Trans, Compiler::Gcc,
            kStackIntThenLdLibTrans, kStackIntThenLdMainGcc, "42"));
}

TEST(SysVAbi, vaArgLongDouble_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_va_ld_tg", Compiler::Gcc, Compiler::Trans,
            kVaArgLongDoubleLibGcc, kVaArgLongDoubleMainGcc, "42"));
}

TEST(SysVAbi, vaArgLongDouble_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_va_ld_gt", Compiler::Trans, Compiler::Gcc,
            kVaArgLongDoubleLibTrans, kVaArgLongDoubleMainTrans, "42"));
}

constexpr const char* kVaAfterNamedLdThenGpLibTrans = R"prg(
        int take(int a, int b, int c, int d, int e, long double ld, int g, ...) {
            __builtin_va_list ap;
            int n;
            __builtin_va_start(ap, g);
            n = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            (void)a;
            (void)b;
            (void)c;
            (void)d;
            (void)e;
            (void)ld;
            (void)g;
            return n;
        }
    )prg";

constexpr const char* kVaAfterNamedLdThenGpMainGcc = R"prg(
        int printf(const char *, ...);
        int take(int, int, int, int, int, long double, int, ...);
        int main(void) {
            printf("%d", take(1, 2, 3, 4, 5, 99.0L, 6, 42));
            return 0;
        }
    )prg";

constexpr const char* kVaAfterNamedLdThenGpLibGcc = R"prg(
        #include <stdarg.h>
        int take(int a, int b, int c, int d, int e, long double ld, int g, ...) {
            va_list ap;
            int n;
            va_start(ap, g);
            n = va_arg(ap, int);
            va_end(ap);
            (void)a;
            (void)b;
            (void)c;
            (void)d;
            (void)e;
            (void)ld;
            (void)g;
            return n;
        }
    )prg";

constexpr const char* kVaAfterNamedLdThenGpMainTrans = R"prg(
        int printf(const char *, ...);
        int take(int, int, int, int, int, long double, int, ...);
        int main(void) {
            printf("%d", take(1, 2, 3, 4, 5, 99.0L, 6, 42));
            return 0;
        }
    )prg";

TEST(SysVAbi, vaArgAfterNamedLdThenGp_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_va_ld_gp_gt", Compiler::Trans, Compiler::Gcc,
            kVaAfterNamedLdThenGpLibTrans, kVaAfterNamedLdThenGpMainGcc, "42"));
}

TEST(SysVAbi, vaArgAfterNamedLdThenGp_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_va_ld_gp_tg", Compiler::Gcc, Compiler::Trans,
            kVaAfterNamedLdThenGpLibGcc, kVaAfterNamedLdThenGpMainTrans, "42"));
}


// Callees that trust SysV 32/64-bit extension (no re-extend from dil/di).
constexpr const char* kNarrowGprLibGcc = R"prg(
        int take_uc32(unsigned char);
        int take_sc32(char);
        int take_us32(unsigned short);
        long take_ui64(unsigned);
        __asm__(
            ".text\n"
            ".globl take_uc32\n"
            "take_uc32:\n"
            "  movl %edi, %eax\n"
            "  ret\n"
            ".globl take_sc32\n"
            "take_sc32:\n"
            "  movl %edi, %eax\n"
            "  ret\n"
            ".globl take_us32\n"
            "take_us32:\n"
            "  movl %edi, %eax\n"
            "  ret\n"
            ".globl take_ui64\n"
            "take_ui64:\n"
            "  movq %rdi, %rax\n"
            "  ret\n"
        );
    )prg";

constexpr const char* kNarrowGprMainTrans = R"prg(
        int printf(const char *, ...);
        int take_uc32(unsigned char);
        int take_sc32(char);
        int take_us32(unsigned short);
        long take_ui64(unsigned);
        int main(void) {
            unsigned char uc[1];
            char sc[1];
            unsigned short us[2];
            unsigned ui;
            long wid;
            uc[0] = 255;
            sc[0] = -1;
            us[0] = 65535;
            us[1] = 1;
            ui = 0;
            ui = ui - 1;
            wid = take_ui64(ui);
            printf("%d %d %d %d", take_uc32(uc[0]), take_sc32(sc[0]), take_us32(us[0]),
                    (int)(wid > 0));
            return 0;
        }
    )prg";

constexpr const char* kNarrowGprLibTrans = R"prg(
        int take_uc(unsigned char c) {
            return c;
        }
        int take_sc(char c) {
            return c;
        }
        int take_us(unsigned short s) {
            return s;
        }
        unsigned take_ui(unsigned u) {
            return u;
        }
    )prg";

constexpr const char* kNarrowGprMainGcc = R"prg(
        int printf(const char *, ...);
        int take_uc(unsigned char);
        int take_sc(char);
        int take_us(unsigned short);
        unsigned take_ui(unsigned);
        int main(void) {
            printf("%d %d %d %d", take_uc(255), take_sc(-1), take_us(65535), (int)take_ui(4294967295u));
            return 0;
        }
    )prg";

TEST(SysVAbi, narrowGprExtend_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_narrow_gpr_tg", Compiler::Gcc, Compiler::Trans,
            kNarrowGprLibGcc, kNarrowGprMainTrans, "255 -1 65535 1"));
}

TEST(SysVAbi, narrowGprExtend_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_narrow_gpr_gt", Compiler::Trans, Compiler::Gcc,
            kNarrowGprLibTrans, kNarrowGprMainGcc, "255 -1 65535 -1"));
}

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

constexpr const char* kComplexFloatIdentLib = R"prg(
        _Complex float ident_cf(_Complex float x) {
            return x;
        }
    )prg";

constexpr const char* kComplexFloatIdentMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float ident_cf(_Complex float);
        int main(void) {
            _Complex float z;
            __real__ z = 1.0f;
            __imag__ z = 2.0f;
            z = ident_cf(z);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatMakeSplitLibGcc = R"prg(
        _Complex float make_cf(void) {
            _Complex float z;
            __real__ z = 1.0f;
            __imag__ z = 2.0f;
            return z;
        }
        int split_cf(_Complex float z) {
            return (int)__real__ z + 10 * (int)__imag__ z;
        }
    )prg";

constexpr const char* kComplexFloatMakeSplitMainTrans = R"prg(
        int printf(const char *, ...);
        _Complex float make_cf(void);
        int split_cf(_Complex float);
        int main(void) {
            printf("%d", split_cf(make_cf()));
            return 0;
        }
    )prg";

TEST(SysVAbi, complexFloatIdent_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_ident_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatIdentLib, kComplexFloatIdentMainGcc, "1 2"));
}

TEST(SysVAbi, complexFloatPassReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_pass_tg", Compiler::Gcc, Compiler::Trans,
            kComplexFloatMakeSplitLibGcc, kComplexFloatMakeSplitMainTrans, "21"));
}

constexpr const char* kComplexDoubleIdentLib = R"prg(
        _Complex double ident_cd(_Complex double x) {
            return x;
        }
    )prg";

constexpr const char* kComplexDoubleIdentMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex double ident_cd(_Complex double);
        int main(void) {
            _Complex double z;
            __real__ z = 3.0;
            __imag__ z = 4.0;
            z = ident_cd(z);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexDoubleMakeSplitLibGcc = R"prg(
        _Complex double make_cd(void) {
            _Complex double z;
            __real__ z = 3.0;
            __imag__ z = 4.0;
            return z;
        }
        int split_cd(_Complex double z) {
            return (int)__real__ z + 10 * (int)__imag__ z;
        }
    )prg";

constexpr const char* kComplexDoubleMakeSplitMainTrans = R"prg(
        int printf(const char *, ...);
        _Complex double make_cd(void);
        int split_cd(_Complex double);
        int main(void) {
            printf("%d", split_cd(make_cd()));
            return 0;
        }
    )prg";

TEST(SysVAbi, complexDoubleIdent_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_ident_gt", Compiler::Trans, Compiler::Gcc,
            kComplexDoubleIdentLib, kComplexDoubleIdentMainGcc, "3 4"));
}

TEST(SysVAbi, complexDoublePassReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_pass_tg", Compiler::Gcc, Compiler::Trans,
            kComplexDoubleMakeSplitLibGcc, kComplexDoubleMakeSplitMainTrans, "43"));
}

constexpr const char* kComplexDoubleSecondLib = R"prg(
        _Complex double second_cd(_Complex double a, _Complex double b) {
            return b;
        }
    )prg";

constexpr const char* kComplexDoubleSecondMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex double second_cd(_Complex double, _Complex double);
        int main(void) {
            _Complex double a;
            _Complex double b;
            __real__ a = 1.0;
            __imag__ a = 2.0;
            __real__ b = 5.0;
            __imag__ b = 6.0;
            a = second_cd(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

TEST(SysVAbi, complexDoubleSecondArg_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_2nd_gt", Compiler::Trans, Compiler::Gcc,
            kComplexDoubleSecondLib, kComplexDoubleSecondMainGcc, "5 6"));
}

constexpr const char* kComplexLongDoubleIdentLib = R"prg(
        _Complex long double ident_cld(_Complex long double x) {
            return x;
        }
    )prg";

constexpr const char* kComplexLongDoubleIdentMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex long double ident_cld(_Complex long double);
        int main(void) {
            _Complex long double z;
            __real__ z = 7.0L;
            __imag__ z = 8.0L;
            z = ident_cld(z);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexLongDoubleMakeSplitLibGcc = R"prg(
        _Complex long double make_cld(void) {
            _Complex long double z;
            __real__ z = 7.0L;
            __imag__ z = 8.0L;
            return z;
        }
        int split_cld(_Complex long double z) {
            return (int)__real__ z + 10 * (int)__imag__ z;
        }
    )prg";

constexpr const char* kComplexLongDoubleMakeSplitMainTrans = R"prg(
        int printf(const char *, ...);
        _Complex long double make_cld(void);
        int split_cld(_Complex long double);
        int main(void) {
            printf("%d", split_cld(make_cld()));
            return 0;
        }
    )prg";

TEST(SysVAbi, complexLongDoubleIdent_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cld_ident_gt", Compiler::Trans, Compiler::Gcc,
            kComplexLongDoubleIdentLib, kComplexLongDoubleIdentMainGcc, "7 8"));
}

constexpr const char* kComplexFloatAddLibTrans = R"prg(
        _Complex float add_cf(_Complex float a, _Complex float b) {
            return a + b;
        }
    )prg";

constexpr const char* kComplexFloatAddMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float add_cf(_Complex float, _Complex float);
        int main(void) {
            _Complex float a;
            _Complex float b;
            __real__ a = 1.0f;
            __imag__ a = 2.0f;
            __real__ b = 3.0f;
            __imag__ b = 4.0f;
            a = add_cf(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatFromRealLibTrans = R"prg(
        _Complex float from_f(float x) {
            return x;
        }
    )prg";

constexpr const char* kComplexFloatFromRealMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float from_f(float);
        int main(void) {
            _Complex float z;
            z = from_f(3.0f);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatToRealLibTrans = R"prg(
        float real_cf(_Complex float z) {
            return z;
        }
    )prg";

constexpr const char* kComplexFloatToRealMainGcc = R"prg(
        int printf(const char *, ...);
        float real_cf(_Complex float);
        int main(void) {
            _Complex float z;
            __real__ z = 1.0f;
            __imag__ z = 2.0f;
            printf("%d", (int)real_cf(z));
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatNegLibTrans = R"prg(
        _Complex float neg_cf(_Complex float z) {
            return -z;
        }
    )prg";

constexpr const char* kComplexFloatNegMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float neg_cf(_Complex float);
        int main(void) {
            _Complex float z;
            __real__ z = 1.0f;
            __imag__ z = 2.0f;
            z = neg_cf(z);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexDoubleAddLibTrans = R"prg(
        _Complex double add_cd(_Complex double a, _Complex double b) {
            return a + b;
        }
    )prg";

constexpr const char* kComplexDoubleAddMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex double add_cd(_Complex double, _Complex double);
        int main(void) {
            _Complex double a;
            _Complex double b;
            __real__ a = 1.0;
            __imag__ a = 2.0;
            __real__ b = 3.0;
            __imag__ b = 4.0;
            a = add_cd(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexLongDoubleAddLibTrans = R"prg(
        _Complex long double add_cld(_Complex long double a, _Complex long double b) {
            return a + b;
        }
    )prg";

constexpr const char* kComplexLongDoubleAddMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex long double add_cld(_Complex long double, _Complex long double);
        int main(void) {
            _Complex long double a;
            _Complex long double b;
            __real__ a = 1.0L;
            __imag__ a = 2.0L;
            __real__ b = 3.0L;
            __imag__ b = 4.0L;
            a = add_cld(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

TEST(SysVAbi, complexFloatAdd_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_add_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatAddLibTrans, kComplexFloatAddMainGcc, "4 6"));
}

TEST(SysVAbi, complexFloatFromRealZerosImag_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_real_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatFromRealLibTrans, kComplexFloatFromRealMainGcc, "3 0"));
}

TEST(SysVAbi, complexFloatToRealDropsImag_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_drop_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatToRealLibTrans, kComplexFloatToRealMainGcc, "1"));
}

TEST(SysVAbi, complexFloatNeg_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_neg_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatNegLibTrans, kComplexFloatNegMainGcc, "-1 -2"));
}

TEST(SysVAbi, complexDoubleAdd_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_add_gt", Compiler::Trans, Compiler::Gcc,
            kComplexDoubleAddLibTrans, kComplexDoubleAddMainGcc, "4 6"));
}

constexpr const char* kComplexFloatMulLibTrans = R"prg(
        _Complex float mul_cf(_Complex float a, _Complex float b) {
            return a * b;
        }
    )prg";

constexpr const char* kComplexFloatMulMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float mul_cf(_Complex float, _Complex float);
        int main(void) {
            _Complex float a;
            _Complex float b;
            __real__ a = 1.0f;
            __imag__ a = 2.0f;
            __real__ b = 3.0f;
            __imag__ b = 4.0f;
            a = mul_cf(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatDivLibTrans = R"prg(
        _Complex float div_cf(_Complex float a, _Complex float b) {
            return a / b;
        }
    )prg";

constexpr const char* kComplexFloatDivMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float div_cf(_Complex float, _Complex float);
        int main(void) {
            _Complex float a;
            _Complex float b;
            __real__ a = 0.0f;
            __imag__ a = 2.0f;
            __real__ b = 1.0f;
            __imag__ b = 1.0f;
            a = div_cf(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexDoubleMulLibTrans = R"prg(
        _Complex double mul_cd(_Complex double a, _Complex double b) {
            return a * b;
        }
    )prg";

constexpr const char* kComplexDoubleMulMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex double mul_cd(_Complex double, _Complex double);
        int main(void) {
            _Complex double a;
            _Complex double b;
            __real__ a = 1.0;
            __imag__ a = 2.0;
            __real__ b = 3.0;
            __imag__ b = 4.0;
            a = mul_cd(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexLongDoubleMulLibTrans = R"prg(
        _Complex long double mul_cld(_Complex long double a, _Complex long double b) {
            return a * b;
        }
    )prg";

constexpr const char* kComplexLongDoubleMulMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex long double mul_cld(_Complex long double, _Complex long double);
        int main(void) {
            _Complex long double a;
            _Complex long double b;
            __real__ a = 1.0L;
            __imag__ a = 2.0L;
            __real__ b = 3.0L;
            __imag__ b = 4.0L;
            a = mul_cld(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

TEST(SysVAbi, complexFloatMul_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_mul_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatMulLibTrans, kComplexFloatMulMainGcc, "-5 10"));
}

TEST(SysVAbi, complexFloatDiv_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_div_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatDivLibTrans, kComplexFloatDivMainGcc, "1 1"));
}

TEST(SysVAbi, complexDoubleMul_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_mul_gt", Compiler::Trans, Compiler::Gcc,
            kComplexDoubleMulLibTrans, kComplexDoubleMulMainGcc, "-5 10"));
}

constexpr const char* kComplexFloatEqLibTrans = R"prg(
        int eq_cf(_Complex float a, _Complex float b) {
            return a == b;
        }
        int ne_cf(_Complex float a, _Complex float b) {
            return a != b;
        }
    )prg";

constexpr const char* kComplexFloatEqMainGcc = R"prg(
        int printf(const char *, ...);
        int eq_cf(_Complex float, _Complex float);
        int ne_cf(_Complex float, _Complex float);
        int main(void) {
            _Complex float a;
            _Complex float b;
            __real__ a = 1.0f;
            __imag__ a = 2.0f;
            __real__ b = 1.0f;
            __imag__ b = 2.0f;
            int same = eq_cf(a, b);
            __imag__ b = 3.0f;
            printf("%d %d", same, ne_cf(a, b));
            return 0;
        }
    )prg";

TEST(SysVAbi, complexFloatEq_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_eq_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatEqLibTrans, kComplexFloatEqMainGcc, "1 1"));
}

TEST(SysVAbi, complexLongDoubleMul_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cld_mul_gt", Compiler::Trans, Compiler::Gcc,
            kComplexLongDoubleMulLibTrans, kComplexLongDoubleMulMainGcc, "-5 10"));
}

TEST(SysVAbi, complexLongDoubleAdd_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cld_add_gt", Compiler::Trans, Compiler::Gcc,
            kComplexLongDoubleAddLibTrans, kComplexLongDoubleAddMainGcc, "4 6"));
}

constexpr const char* kBitFieldPackLib = R"prg(
        struct Pack {
            int a:4;
            int b:4;
            int c:8;
        };
        int sum_pack(struct Pack p) {
            return p.a + p.b + p.c;
        }
    )prg";

constexpr const char* kBitFieldPackMain = R"prg(
        int printf(const char *, ...);
        struct Pack {
            int a:4;
            int b:4;
            int c:8;
        };
        int sum_pack(struct Pack p);
        int main(void) {
            struct Pack p;
            p.a = 1;
            p.b = 2;
            p.c = 3;
            printf("%d", sum_pack(p));
            return 0;
        }
    )prg";

constexpr const char* kBitFieldMakeLib = R"prg(
        struct Pack {
            int a:4;
            int b:4;
            int c:8;
        };
        struct Pack make_pack(void) {
            struct Pack p;
            p.a = 1;
            p.b = 2;
            p.c = 3;
            return p;
        }
    )prg";

constexpr const char* kBitFieldMakeMain = R"prg(
        int printf(const char *, ...);
        struct Pack {
            int a:4;
            int b:4;
            int c:8;
        };
        struct Pack make_pack(void);
        int main(void) {
            struct Pack p;
            p = make_pack();
            printf("%d %d %d", p.a, p.b, p.c);
            return 0;
        }
    )prg";

TEST(SysVAbi, bitFieldPack_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_bf_pack_gt", Compiler::Trans, Compiler::Gcc,
            kBitFieldPackLib, kBitFieldPackMain, "6"));
}

TEST(SysVAbi, bitFieldPack_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_bf_pack_tg", Compiler::Gcc, Compiler::Trans,
            kBitFieldPackLib, kBitFieldPackMain, "6"));
}

TEST(SysVAbi, bitFieldReturn_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_bf_ret_gt", Compiler::Trans, Compiler::Gcc,
            kBitFieldMakeLib, kBitFieldMakeMain, "1 2 3"));
}

TEST(SysVAbi, bitFieldReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_bf_ret_tg", Compiler::Gcc, Compiler::Trans,
            kBitFieldMakeLib, kBitFieldMakeMain, "1 2 3"));
}

TEST(SysVAbi, complexLongDoublePassReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cld_pass_tg", Compiler::Gcc, Compiler::Trans,
            kComplexLongDoubleMakeSplitLibGcc, kComplexLongDoubleMakeSplitMainTrans, "87"));
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

TEST(SysVAbi, sseThenIntegerReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_sse_int_ret_tg", Compiler::Gcc, Compiler::Trans,
            kSseThenIntegerReturnLib, kSseThenIntegerReturnMain, "42 99"));
}

TEST(SysVAbi, sseThenIntegerReturn_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_sse_int_ret_gt", Compiler::Trans, Compiler::Gcc,
            kSseThenIntegerReturnLib, kSseThenIntegerReturnMain, "42 99"));
}

} // namespace
