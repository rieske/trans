#include "TestFixtures.h"

#include "util/Process.h"
#include "NmSymbols.h"

namespace {

void expectObjectDefines(const std::string& objectPath, const std::string& name) {
    util::ProcessResult result = util::runProcess({ "nm", "-P", objectPath });
    ASSERT_EQ(result.exitCode, 0) << result.stderrOutput;
    EXPECT_TRUE(nmTypeIsDefined(nmSymbolType(result.stdoutOutput, name))) << result.stdoutOutput;
}

TEST(Compiler, externIncompleteArrayThenCompleteDefinition) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern char a[];
        char a[1];
        int main(void) {
            printf("%d %d", (int)sizeof a, a[0]);
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDefines(program.getSourceFilePath() + ".o", "a");
    program.runAndExpect("1 0");
}

TEST(Compiler, completeArrayThenExternIncomplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char a[4];
        extern char a[];
        int main(void) {
            printf("%d", (int)sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, externIncompleteArrayThenExplicitInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern char a[];
        char a[4] = { 1, 2, 3, 4 };
        int main(void) {
            printf("%d %d %d", a[0], a[3], (int)sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDefines(program.getSourceFilePath() + ".o", "a");
    program.runAndExpect("1 4 4");
}

TEST(Compiler, externIncompleteConstArrayThenComplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern const char a[];
        const char a[3] = "hi";
        int main(void) {
            printf("%d %d %d %d", a[0], a[1], a[2], (int)sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3");
}

TEST(Compiler, externIncompleteMultidimArrayThenComplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern int a[][3];
        int a[2][3];
        int main(void) {
            printf("%d", (int)sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("24");
}

TEST(Compiler, pointerToIncompleteArrayThenComplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int (*p)[];
        int (*p)[4];
        int main(void) {
            printf("%d", (int)sizeof(*p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16");
}

TEST(Compiler, incompleteArrayIndexedBeforeCompletingDeclaration) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern char a[];
        int first(void) {
            return a[0];
        }
        char a[1] = { 7 };
        int main(void) {
            printf("%d %d", first(), (int)sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 1");
}

TEST(Compiler, gitShapedSlopbufRedecl) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern char slop[];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        #define STRBUF_INIT { .buf = slop }
        struct strbuf g = STRBUF_INIT;
        char slop[1];
        int main(void) {
            slop[0] = 65;
            printf("%d %d %d %d", (int)sizeof slop, g.alloc == 0, g.buf == slop, g.buf[0]);
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDefines(program.getSourceFilePath() + ".o", "slop");
    program.runAndExpect("1 1 1 65");
}

TEST(Compiler, functionPointerRedeclSameType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern int (*fp)(void);
        int f(void) {
            return 3;
        }
        int (*fp)(void) = f;
        int main(void) {
            printf("%d", fp());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, functionPointerParamPointerToIncompleteArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int (*gp)(int (*)[]);
        int take(int (*p)[4]) {
            return (*p)[0] + (*p)[3];
        }
        int (*gp)(int (*)[4]) = take;
        int main(void) {
            int a[4] = { 1, 2, 3, 4 };
            printf("%d", gp(&a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, incompleteArrayOfPointerToIncompleteArrayThenInnerComplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern int (*rows[])[];
        extern int (*rows[])[4];
        int row[4] = { 9, 8, 7, 6 };
        int (*rows[1])[4] = { &row };
        int main(void) {
            printf("%d %d", (int)sizeof rows, (*rows[0])[2]);
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDefines(program.getSourceFilePath() + ".o", "rows");
    program.runAndExpect("8 7");
}

} // namespace
