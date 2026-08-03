#include "SysVAbiInteropHarness.h"
#include "TestFixtures.h"

namespace {

using sysv_abi_interop::Compiler;
using sysv_abi_interop::linkRunExpect;

TEST(Compiler, builtinAllocaWriteRead) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int *p;
            p = __builtin_alloca(2 * sizeof(int));
            p[0] = 7;
            p[1] = 9;
            printf("%d %d", p[0], p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 9");
}

TEST(Compiler, builtinAllocaRuntimeSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int n;
            char *p;
            n = 4;
            p = __builtin_alloca(n);
            p[0] = 65;
            p[3] = 66;
            printf("%d %d", p[0], p[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65 66");
}

TEST(Compiler, builtinAllocaKeepsLocals) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int a;
            char *p;
            int b;
            a = 1;
            b = 2;
            p = __builtin_alloca(8);
            p[0] = 3;
            printf("%d %d %d", a, b, p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, builtinAllocaLocalsSurviveStackArgCall) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int take7(int a, int b, int c, int d, int e, int f, int g) {
            return a + b + c + d + e + f + g;
        }
        int main(void) {
            int x;
            char *p;
            x = 10;
            p = __builtin_alloca(8);
            p[0] = 1;
            printf("%d %d", take7(x, x, x, x, x, x, x), p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("70 1");
}

TEST(Compiler, builtinAllocaGitFastArrayShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int n;
            int **p;
            n = 2;
            if (n <= 2) {
                p = __builtin_alloca(n * sizeof(*p));
            } else {
                p = 0;
            }
            p[0] = 0;
            p[1] = 0;
            printf("%d", p != 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, builtinAllocaSixteenByteAligned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            char *p;
            p = __builtin_alloca(1);
            printf("%d", ((unsigned long)p & 15) == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, builtinAllocaWrongArityIsError) {
    SourceProgram program{R"prg(
        int main(void) {
            void *p;
            p = __builtin_alloca();
            return p == 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("wrong number of arguments to __builtin_alloca");
}

TEST(Compiler, builtinAllocaCalleePreservesCallerLocals) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int inner(void) {
            char *p;
            p = __builtin_alloca(64);
            p[0] = 1;
            return 1;
        }
        int main(void) {
            int a0; int a1; int a2; int a3; int a4;
            int a5; int a6; int a7; int a8; int a9;
            int r;
            a0 = 1; a1 = 2; a2 = 3; a3 = 4; a4 = 5;
            a5 = 6; a6 = 7; a7 = 8; a8 = 9; a9 = 10;
            r = inner();
            printf("%d %d", r, a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 55");
}

TEST(SysVAbi, alloca_gccCallerKeepsRbxAcrossTransAlloca) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_alloca_callee_saved", Compiler::Trans, Compiler::Gcc,
            R"prg(
                int inner(void) {
                    char *p;
                    p = __builtin_alloca(64);
                    p[0] = 1;
                    return 1;
                }
            )prg",
            R"prg(
                int printf(const char *, ...);
                long g = 0x1122334455667788;
                int inner(void);
                int main(void) {
                    long v;
                    int r;
                    v = g;
                    r = inner();
                    printf("%d %lx", r, (unsigned long)v);
                    return 0;
                }
            )prg",
            "1 1122334455667788"));
}

TEST(Compiler, isoStdRejectsBuiltinAlloca) {
    SourceProgram program{R"prg(
        int main(void) {
            void *p;
            p = __builtin_alloca(8);
            return p == 0;
        }
    )prg", std::vector<std::string> { "-std=c" }};
    program.compile();
    program.assertCompilationErrors("symbol `__builtin_alloca` is not defined");
}

} // namespace
