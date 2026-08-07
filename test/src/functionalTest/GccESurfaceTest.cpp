#include "TestFixtures.h"

namespace {

TEST(Compiler, leftoverPragmaDoesNotBreakParse) {
    SourceProgram program{R"prg(
        #pragma once
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, boolTypedefStandInIsOneByte) {
    SourceProgram program{R"prg(
        int main() {
            _Bool b;
            b = 1;
            printf("%d %d", (int)sizeof(_Bool), (int)b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, int128TypedefStandInIsLong) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", (int)sizeof(__int128));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, float64TypedefStandInIsDouble) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", (int)sizeof(_Float64));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, inlineFunctionIsAccepted) {
    SourceProgram program{R"prg(
        static inline int add1(int x) {
            return x + 1;
        }
        int main() {
            printf("%d", add1(41));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, restrictPointerParamIsAccepted) {
    SourceProgram program{R"prg(
        int load(const int * restrict p) {
            return *p;
        }
        int main() {
            int x;
            x = 7;
            printf("%d", load(&x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, funcNameIsCurrentFunction) {
    SourceProgram program{R"prg(
        int main() {
            printf("%s", __func__);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("main");
}

TEST(Compiler, attributeAfterDeclaratorIsIgnored) {
    SourceProgram program{R"prg(
        int x __attribute__((unused));
        int main() {
            x = 3;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, adjacentStringsConcatenate) {
    SourceProgram program{R"prg(
        int main() {
            printf("%s", "ab" "cd");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("abcd");
}

TEST(Compiler, wideStringPrefixIsPlainString) {
    SourceProgram program{R"prg(
        int main() {
            printf("%s", L"ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, statementAsmIsNullStatement) {
    SourceProgram program{R"prg(
        int main() {
            asm volatile ("nop");
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, statementAsmGnuVolatileAndExtensionIsNullStatement) {
    SourceProgram program{R"prg(
        int main() {
            asm __volatile__ ("nop");
            asm __extension__ volatile ("nop");
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, asmBareSpellingOnDeclarator) {
    SourceProgram program{R"prg(
        int y __asm("x");
        int main() {
            y = 3;
            printf("%d", y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, adjacentStringsConcatenateAcrossExtension) {
    SourceProgram program{R"prg(
        int main() {
            printf("%s", "ab" __extension__ "cd");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("abcd");
}

TEST(Compiler, funcNameOutsideFunctionIsError) {
    SourceProgram program{R"prg(
        const char *p = __func__;
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("__func__ used outside a function");
}

} // namespace
