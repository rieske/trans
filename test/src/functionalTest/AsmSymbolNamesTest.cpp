#include "TestFixtures.h"

#include <string>

namespace {

std::size_t countSubstr(const std::string& haystack, const std::string& needle) {
    std::size_t n = 0;
    for (std::size_t pos = 0; (pos = haystack.find(needle, pos)) != std::string::npos;
            pos += needle.size()) {
        ++n;
    }
    return n;
}

TEST(Compiler, callLibcAbs) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int abs(int);
        int main(void) {
            printf("%d", abs(-3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, userFunctionNamedMov) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int mov(void) {
            return 7;
        }
        int main(void) {
            printf("%d", mov());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, globalNamedNegAndDq) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int neg;
        int dq;
        int main(void) {
            neg = 4;
            dq = 5;
            printf("%d %d", neg, dq);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 5");
}

TEST(Compiler, globalNamedRax) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int rax;
        int main(void) {
            rax = 9;
            printf("%d", rax);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, fileScopePointerToAbs) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int abs(int);
        int (*fp)(int) = abs;
        int main(void) {
            printf("%d", fp(-4));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, fileScopeStringPointerIsNotExtern) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *p = "hi";
        int main(void) {
            printf("%s", p);
            return 0;
        }
    )prg"};
    program.addCompilerArg("-save-temps");
    program.compile();
    EXPECT_THAT(program.readAssembly(), Not(HasSubstr("extern L$str")));
    program.runAndExpect("hi");
}

TEST(Compiler, unusedExternObjectIsDeclared) {
    SourceProgram program{R"prg(
        extern int x;
        int main(void) {
            return 0;
        }
    )prg"};
    program.addCompilerArg("-save-temps");
    program.compile();
    EXPECT_THAT(countSubstr(program.readAssembly(), "extern x\n"), Eq(1u));
    program.runAndExpect("");
}

TEST(Compiler, fileScopePointerToExternDataEmitsOneExtern) {
    SourceProgram program{R"prg(
        extern int x;
        int *p = &x;
        int main(void) {
            return 0;
        }
    )prg"};
    program.addCompilerArg("-c");
    program.addCompilerArg("-save-temps");
    program.compile();
    EXPECT_THAT(countSubstr(program.readAssembly(), "extern x\n"), Eq(1u));
}

TEST(Compiler, defineAbsAndCallIt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int abs(int x) {
            if (x < 0) {
                return 0 - x;
            }
            return x;
        }
        int main(void) {
            printf("%d %d", abs(-8), abs(2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 2");
}

} // namespace
