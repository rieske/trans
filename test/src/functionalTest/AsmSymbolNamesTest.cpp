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
    )prg", {"-save-temps"}};
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
    )prg", {"-save-temps"}};
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
    )prg", {"-c", "-save-temps"}};
    program.compile();
    EXPECT_THAT(countSubstr(program.readAssembly(), "extern x\n"), Eq(1u));
}

TEST(Compiler, zeroInitializedGlobalsAreBss) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int zeroArr[32];
        int explicitZero = 0;
        int nonzero = 7;
        int mixed[2] = {1, 0};
        int *nullp = 0;
        int *addr = &nonzero;
        int main(void) {
            printf("%d %d %d %d %d %d %d",
                zeroArr[0], zeroArr[31], explicitZero, nonzero,
                mixed[0] + mixed[1], nullp == 0, addr == &nonzero);
            return 0;
        }
    )prg", {"-save-temps"}};
    program.compile();
    const std::string asmText = program.readAssembly();
    const bool intel = functionalTestDialectTag() == "intel";
    EXPECT_THAT(asmText, HasSubstr(intel ? "section .bss" : ".section .bss"));
    EXPECT_THAT(asmText, HasSubstr(intel ? "resb " : ".zero "));
    EXPECT_THAT(asmText, HasSubstr(intel ? "section .data" : ".section .data"));
    EXPECT_THAT(asmText, Not(HasSubstr("0, 0, 0, 0")));
    program.runAndExpect("0 0 0 7 1 1 1");
}

// A 12-byte struct copy moves two 8-byte words. The int after the destination
// must stay put; the old .data tail pad used to absorb that extra word.
TEST(Compiler, structCopyIntoGlobalDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int a; int b; char c; };
        struct S src;
        int srcTail;
        struct S dst;
        int dstTail;
        int main(void) {
            src.a = 1;
            src.b = 2;
            src.c = 3;
            srcTail = 287454020;
            dstTail = 85;
            dst = src;
            printf("%d %d %d %d %d", dst.a, dst.b, (int)dst.c, dstTail, srcTail);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 85 287454020");
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
