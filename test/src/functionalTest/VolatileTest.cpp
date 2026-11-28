#include "TestFixtures.h"

namespace {

void expectKeeps(const char* source, const char* output, const char* intel, const char* att) {
    SourceProgram program { source, { "-save-temps" } };
    program.compile();
    program.runAndExpect(output);
    EXPECT_THAT(program.readAssembly(), AnyOf(HasSubstr(intel), HasSubstr(att)));
}

TEST(Compiler, volatileAddReloads) {
    expectKeeps(R"prg(int printf(const char *, ...);
        int main(void) {
            volatile int x;
            int y;
            x = 1;
            y = x + 1;
            printf("%d", y);
            return 0;
        }
    )prg", "2", "add ", "addl");
}

TEST(Compiler, volatileSubtractSelfReloads) {
    expectKeeps(R"prg(int printf(const char *, ...);
        int main(void) {
            volatile int x;
            int y;
            x = 1;
            y = x - x;
            printf("%d", y);
            return 0;
        }
    )prg", "0", "sub eax", "subl");
}

TEST(Compiler, volatileMultiplyByZeroReloads) {
    expectKeeps(R"prg(int printf(const char *, ...);
        int main(void) {
            volatile int x;
            int y;
            x = 1;
            y = x * 0;
            printf("%d", y);
            return 0;
        }
    )prg", "0", "imul ", "imull");
}

std::string mainAssembly(const std::string& text) {
    const auto at = text.rfind("main:");
    if (at == std::string::npos) {
        return {};
    }
    return text.substr(at);
}

void expectInlinedVolatileSubtract(const char* source) {
    SourceProgram program { source, { "-save-temps" } };
    program.compile();
    program.runAndExpect("0");
    const std::string main = mainAssembly(program.readAssembly());
    if (functionalTestOptFlag() == "-O0") {
        EXPECT_THAT(main, HasSubstr("diff"));
        return;
    }
    EXPECT_THAT(main, AnyOf(HasSubstr("sub e"), HasSubstr("subl")));
}

TEST(Compiler, inlinedVolatileLocalSubtracts) {
    expectInlinedVolatileSubtract(R"prg(int printf(const char *, ...);
        static int diff(void) {
            volatile int x;
            int y;
            x = 1;
            y = x - x;
            return y;
        }
        int main(void) {
            printf("%d", diff());
            return 0;
        }
    )prg");
}

TEST(Compiler, inlinedVolatileParameterSubtracts) {
    expectInlinedVolatileSubtract(R"prg(int printf(const char *, ...);
        static int diff(volatile int x) {
            return x - x;
        }
        int main(void) {
            int a;
            a = 1;
            printf("%d", diff(a));
            return 0;
        }
    )prg");
}

TEST(Compiler, plainAddStillFolds) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int main(void) {
            int x;
            int y;
            x = 1;
            y = x + 1;
            printf("%d", y);
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, volatileLocalKeepsValueAcrossLongjmp) {
    SourceProgram program { R"prg(#include <setjmp.h>
        int printf(const char *, ...);
        jmp_buf jb;
        int main(void) {
            volatile long seed = 1;
            long a = seed + 99;
            volatile long x = seed;
            long y = a + x;
            if (setjmp(jb) == 0) {
                x = x + 41;
                longjmp(jb, 1);
            }
            printf("%ld %ld", y, x);
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("101 42");
}

TEST(Compiler, volatileParameterReadsIncomingValue) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        static long twice(volatile long x) { return x + x; }
        int main(void) {
            long (*volatile call)(volatile long) = twice;
            printf("%ld", call(21));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("42");
}

} // namespace
