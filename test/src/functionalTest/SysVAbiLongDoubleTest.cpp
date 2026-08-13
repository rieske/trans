#include "SysVAbiInteropHarness.h"

namespace {

using sysv_abi_interop::Compiler;
using sysv_abi_interop::linkRunExpect;

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
} // namespace
