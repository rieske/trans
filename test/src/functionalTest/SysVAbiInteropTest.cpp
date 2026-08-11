#include "TestFixtures.h"

#include "util/Process.h"
#include "DriverHarness.h"
#include "ResourceHelpers.h"

#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

enum class Compiler { Trans, Gcc };

std::string dialectStem(const std::string& base) {
    return base + "_" + functionalTestDialectTag();
}

std::string writeTmpC(const std::string& stem, const std::string& body) {
    return writeTempSource(dialectStem(stem), body);
}

std::string readFile(const std::string& path) {
    std::ifstream in { path };
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void removePath(const std::string& path) {
    unlink(path.c_str());
}

std::vector<std::string> dialectFlags(std::vector<std::string> flags) {
    flags.insert(flags.begin(), "-a" + functionalTestDialectTag());
    return flags;
}

int compileTrans(const std::string& sourcePath, const std::string& objectPath, std::string* errOut) {
    ArgvBuffer args { { sourcePath }, dialectFlags({ "-c", "-o" + objectPath }) };
    return runDriver(args, errOut);
}

int compileGcc(const std::string& sourcePath, const std::string& objectPath, std::string* errOut) {
    // -O2 so gcc does not leave the other class's bits in rdx / on the
    // outgoing stack slot (O0 epilogues made mixed returns look correct).
    util::ProcessResult result = util::runProcess({
            "gcc", "-c", "-O2", "-fPIE", "-m64", "-o", objectPath, sourcePath
    });
    if (errOut) {
        *errOut = result.stderrOutput;
    }
    return result.exitCode;
}

int hostLink(const std::vector<std::string>& objects, const std::string& exe) {
    std::vector<std::string> argv { "gcc", "-m64", "-pie", "-o", exe };
    argv.insert(argv.end(), objects.begin(), objects.end());
    return util::runProcess(argv).exitCode;
}

int runExe(const std::string& exe, const std::string& outputFile) {
    removePath(outputFile);
    return util::runProcess({ exe }, {}, outputFile).exitCode;
}

void linkRunExpect(const std::string& stem, Compiler libCompiler, Compiler mainCompiler,
        const std::string& libBody, const std::string& mainBody, const std::string& expected) {
    const std::string libSrc = writeTmpC(stem + "_lib", libBody);
    const std::string mainSrc = writeTmpC(stem + "_main", mainBody);
    const std::string tmp = getTestResourcePath("programs/tmp/");
    const std::string libObj = tmp + dialectStem(stem + "_lib") + ".o";
    const std::string mainObj = tmp + dialectStem(stem + "_main") + ".o";
    const std::string exe = tmp + dialectStem(stem) + ".out";
    const std::string outputFile = exe + ".execution.output";
    removePath(libObj);
    removePath(mainObj);
    removePath(exe);
    removePath(outputFile);

    std::string err;
    const int libRc = libCompiler == Compiler::Gcc
            ? compileGcc(libSrc, libObj, &err)
            : compileTrans(libSrc, libObj, &err);
    ASSERT_EQ(libRc, 0) << err;
    err.clear();
    const int mainRc = mainCompiler == Compiler::Gcc
            ? compileGcc(mainSrc, mainObj, &err)
            : compileTrans(mainSrc, mainObj, &err);
    ASSERT_EQ(mainRc, 0) << err;
    ASSERT_EQ(hostLink({ mainObj, libObj }, exe), 0);
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq(expected));
}

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

} // namespace

