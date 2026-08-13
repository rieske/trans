#include "TestFixtures.h"

#include "util/Process.h"
#include "NmSymbols.h"

#include <fstream>
#include <iterator>

namespace {

void expectObjectDoesNotDefine(const std::string& objectPath, const std::string& name) {
    util::ProcessResult result = util::runProcess({ "nm", "-P", objectPath });
    ASSERT_EQ(result.exitCode, 0) << result.stderrOutput;
    EXPECT_FALSE(nmTypeIsDefined(nmSymbolType(result.stdoutOutput, name))) << result.stdoutOutput;
}

void expectObjectDefines(const std::string& objectPath, const std::string& name) {
    util::ProcessResult result = util::runProcess({ "nm", "-P", objectPath });
    ASSERT_EQ(result.exitCode, 0) << result.stderrOutput;
    EXPECT_TRUE(nmTypeIsDefined(nmSymbolType(result.stdoutOutput, name))) << result.stdoutOutput;
}

TEST(Compiler, externObjectIsNotDefinedInObjectFile) {
    SourceProgram program{R"prg(
        extern int x;
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDoesNotDefine(program.getSourceFilePath() + ".o", "x");
    program.runAndExpect("");
}

TEST(Compiler, repeatedExternObjectIsNotDefined) {
    SourceProgram program{R"prg(
        extern int x;
        extern int x;
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDoesNotDefine(program.getSourceFilePath() + ".o", "x");
    program.runAndExpect("");
}

TEST(Compiler, externThenDefiningDeclaration) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern int x;
        int x = 7;
        int main(void) {
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDefines(program.getSourceFilePath() + ".o", "x");
    program.runAndExpect("7");
}

TEST(Compiler, definingThenExternDeclaration) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int x = 7;
        extern int x;
        int main(void) {
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, tentativeThenDefiningDeclaration) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int x;
        int x = 9;
        int main(void) {
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, externArrayThenPartialInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern const unsigned char t[4];
        int index_of(int i) {
            return (int)t[i];
        }
        const unsigned char t[4] = { 1, 2 };
        int main(void) {
            printf("%d %d %d %d", index_of(0), index_of(1), index_of(2), index_of(3));
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDefines(program.getSourceFilePath() + ".o", "t");
    program.runAndExpect("1 2 0 0");
}

TEST(Compiler, staticThenMatchingExternKeepsDefinition) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int x = 4;
        extern int x;
        int main(void) {
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, externIncompleteStructObjectCompiles) {
    SourceProgram program{R"prg(
        struct S;
        extern struct S s;
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDoesNotDefine(program.getSourceFilePath() + ".o", "s");
    program.runAndExpect("");
}

TEST(Compiler, functionPrototypeIsNotDefinedAsData) {
    SourceProgram program{R"prg(
        int foo(int);
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDoesNotDefine(program.getSourceFilePath() + ".o", "foo");
    program.runAndExpect("");
}

TEST(Compiler, unsizedGlobalArrayCompletedFromInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);

        int a[] = { 7, 8 };
        int main() {
            printf("%d %d %d", a[0], a[1], sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8 8");
}

TEST(Compiler, unsizedGlobalCharArrayCompletedFromStringLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);

        char g[] = "hi";
        int main() {
            printf("%d %d %d %d", g[0], g[1], g[2], sizeof g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3");
}

TEST(Compiler, sizedGlobalCharArrayFromStringLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);

        char g[3] = "hi";
        int main() {
            printf("%d %d %d %d", g[0], g[1], g[2], sizeof g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3");
}

TEST(Compiler, globalCharArrayStringInitIsNotInternedAsStringConstant) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);

        char g[] = "xyzzy_global_no_pool_9f3a";
        int main() {
            printf("%d", sizeof g);
            return 0;
        }
    )prg"};
    program.addCompilerArg("-save-temps");
    program.compile();
    program.runAndExpect("26");
    EXPECT_THAT(program.readAssembly(), Not(HasSubstr("xyzzy_global_no_pool_9f3a")));
}

TEST(Compiler, externIncompleteArrayIsNotDefinedInObjectFile) {
    SourceProgram program{R"prg(
        extern int a[];
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    expectObjectDoesNotDefine(program.getSourceFilePath() + ".o", "a");
    program.runAndExpect("");
}

TEST(Compiler, globalAssignedInMain) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g;

        int main() {
            g = 3;
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}


// File-scope pointer initialized to the address of another global (NASM: p dq g).
TEST(Compiler, globalPointerAddressInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 42;
        int *p = &g;
        int main() {
            printf("%d %d", *p, p == &g);
            return 0;
        }
    )prg", "global_addr_init"};
    program.addCompilerArg("-save-temps");
    program.compile();
    program.runAndExpect("42 1");

    // Assembly must materialize the address of g, not a null/zero literal.
    const std::string asmText = program.readAssembly();
    if (functionalTestDialect() == AssemblyDialect::AtAndT) {
        EXPECT_THAT(asmText, HasSubstr(".quad g"));
    } else {
        EXPECT_THAT(asmText, HasSubstr("dq $g"));
    }
}


// File-scope char array initialized from a string literal (git version strings).
TEST(Compiler, globalCharArrayStringInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        const char msg[] = "hi";
        int main() {
            printf("%s", msg);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("hi");
}


// File-scope pointer to string literal must store the address of the string,
// not the string bytes as the variable (git: const char *blob_type = "blob").
// Without this, *blob_type / strlen(blob_type) treat the first word of "blob"
// as a pointer and crash.
TEST(Compiler, globalPointerToStringLiteral) {
    SourceProgram program{R"prg(#include <stdio.h>
        const char *blob_type = "blob";
        int main() {
            printf("%s %c", blob_type, blob_type[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("blob b");
}


// Same pattern with static (git blob.c is a non-static global; static must work too).
TEST(Compiler, staticGlobalPointerToStringLiteral) {
    SourceProgram program{R"prg(#include <stdio.h>
        static const char *type = "commit";
        int main() {
            printf("%s", type);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("commit");
}


// Local pointer initialized from a global array must get &arr[0], not arr's first word.
// git config: const char *bomptr = utf8_bom;
TEST(Compiler, localPointerInitFromGlobalArray) {
    SourceProgram program{R"prg(#include <stdio.h>
        const char msg[] = "hi";
        int main() {
            const char *p = msg;
            printf("%s %d", p, p == msg);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("hi 1");
}


// Same decay for assignment (already worked) and comparison against the array object.
TEST(Compiler, pointerComparedToGlobalArray) {
    SourceProgram program{R"prg(#include <stdio.h>
        const char msg[] = "ab";
        int main() {
            const char *p;
            p = msg;
            if (p != msg) {
                printf("ne");
                return 0;
            }
            p = p + 1;
            if (p == msg) {
                printf("eq");
                return 0;
            }
            printf("%c ok", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("b ok");
}


// Incomplete array extern then definition with string (version.h + version.c).
TEST(Compiler, globalIncompleteArrayThenStringDefinition) {
    SourceProgram program{R"prg(#include <stdio.h>
        extern const char msg[];
        const char msg[] = "ok";
        int main() {
            printf("%s", msg);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}


TEST(Compiler, globalSharedAcrossFunctions) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g;

        void set() {
            g = 7;
            return;
        }

        int main() {
            set();
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}


TEST(Compiler, globalReadBeforeWriteIsZero) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}


TEST(Compiler, localShadowsGlobal) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g;

        int main() {
            g = 5;
            {
                int g;
                g = 1;
                printf("%d", g);
            }
            printf(" %d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 5");
}


TEST(Compiler, globalUpdatedFromCalleeVisibleInCaller) {
    SourceProgram program{R"prg(#include <stdio.h>
        int counter;

        int inc() {
            counter = counter + 1;
            return counter;
        }

        int main() {
            printf("%d ", inc());
            printf("%d", counter);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}


TEST(Compiler, globalInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 5;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}


TEST(Compiler, globalInitializerExpression) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 2 + 3;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}


TEST(Compiler, globalInitializerNestedExpression) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = (1 + 2) * 4 - 1;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}


TEST(Compiler, globalInitializerShift) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 1 << 4;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16");
}


TEST(Compiler, globalInitializerBitwise) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = (5 & 3) | 8;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}


TEST(Compiler, globalInitializerComparison) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 2 < 3;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}


TEST(Compiler, globalInitializerCharConstant) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 'A';

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65");
}

// Character escape folding and unary folds in constant global initializers.
// Avoid NASM-reserved names (e.g. `neg`, `dq`) as global labels.
TEST(Compiler, globalInitializerConstantFolds) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g_nl = '\n';
        int g_tab = '\t';
        int g_cr = '\r';
        int g_nul = '\0';
        int g_bs = '\\';
        int g_sq = '\'';
        int g_neg = -42;
        int g_pos = +7;
        int g_not0 = !0;
        int g_not5 = !5;
        int g_dbl = -(-3);
        int g_paren = -(1 + 2);

        int main() {
            printf("%d %d %d %d %d %d ", g_nl, g_tab, g_cr, g_nul, g_bs, g_sq);
            printf("%d %d %d %d %d %d", g_neg, g_pos, g_not0, g_not5, g_dbl, g_paren);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 9 13 0 92 39 -42 7 1 0 3 -3");
}

// Remaining C escapes used by ConstantExpression (Coveralls gap vs master).
TEST(Compiler, globalInitializerCharEscapesAbfvAndQuote) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g_a = '\a';
        int g_b = '\b';
        int g_f = '\f';
        int g_v = '\v';
        int g_dq = '\"';
        int g_hex = '\x1b';
        int g_oct = '\033';

        int main() {
            printf("%d %d %d %d %d %d %d", g_a, g_b, g_f, g_v, g_dq, g_hex, g_oct);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8 12 11 34 27 27");
}

TEST(Compiler, twoGlobalsInOneExpression) {
    SourceProgram program{R"prg(#include <stdio.h>
        int a;
        int b;

        int main() {
            a = 2;
            b = 40;
            printf("%d", a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}


TEST(Compiler, globalVariableNameCollidesWithFunctionRejected) {
    SourceProgram program{R"prg(
        int foo;

        int foo() {
            return 0;
        }

        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("conflicts with global variable");
}

// An initialized local that shadows a global must not overwrite the global's value.
TEST(Compiler, initializedLocalShadowingGlobalKeepsGlobalValue) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 1;

        int getGlobal() {
            return g;
        }

        int main() {
            int g = 2;
            printf("%d %d", g, getGlobal());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}


// A local that shadows a global may have a non-constant initializer (it is not a global initializer).
TEST(Compiler, localShadowingGlobalAllowsNonConstantInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 1;

        int f(int p) {
            int g = p;
            return g;
        }

        int main() {
            printf("%d", f(7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}


// A leading-zero literal is octal in C; the global initializer must fold in the right base.
TEST(Compiler, globalInitializerOctal) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 010;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}


// Logical && is a constant expression; a global initializer using it must fold to 0/1.
TEST(Compiler, globalInitializerLogicalAnd) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 1 && 0;
        int h = 2 && 3;

        int main() {
            printf("%d %d", g, h);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}


// Logical || is a constant expression; a global initializer using it must fold to 0/1.
TEST(Compiler, globalInitializerLogicalOr) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g = 0 || 0;
        int h = 0 || 5;

        int main() {
            printf("%d %d", g, h);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

// Address-of nested member for global init.
TEST(Compiler, globalAddressOfMemberInit) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S { int x; int y; };
        struct S g = { 1, 2 };
        int *p = &g.y;
        int main() {
            printf("%d", *p + 40);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}


} // namespace
