#include "TestFixtures.h"

namespace {

TEST(Compiler, enumBasicValues) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            enum Color { RED, GREEN, BLUE };
            printf("%d %d %d", RED, GREEN, BLUE);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 2");
}

TEST(Compiler, enumExplicitValues) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            enum { A = 10, B, C = 20, D };
            printf("%d %d %d %d", A, B, C, D);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 11 20 21");
}

TEST(Compiler, enumVariable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            enum Color { RED, GREEN, BLUE };
            enum Color c;
            c = GREEN;
            printf("%d", c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, enumInArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            enum { ONE = 1, TWO = 2 };
            printf("%d", ONE + TWO);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, enumGlobal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        enum Status { OK, ERR };
        int main() {
            printf("%d %d", OK, ERR);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, enumAsFunctionArgument) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int f(int x) {
            return x + 1;
        }
        int main() {
            enum { N = 41 };
            printf("%d", f(N));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// C99 trailing comma after last enumerator (common in system headers, e.g. idtype_t).
TEST(Compiler, enumTrailingComma) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            enum { A, B, C, };
            printf("%d %d %d", A, B, C);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 2");
}

// System headers use aliases like _SC_IOV_MAX = _SC_UIO_MAXIOV.
TEST(Compiler, enumInitializerReferencesPriorEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            enum {
                A = 7,
                B = A,
                C,
                D = B + 10
            };
            printf("%d %d %d %d", A, B, C, D);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 7 8 17");
}

TEST(Compiler, enumNamedTypeReference) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        enum Color { RED, GREEN };
        int main() {
            enum Color c;
            c = RED;
            printf("%d", c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, enumComparison) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            enum { NO, YES };
            int x;
            x = YES;
            if (x == YES) {
                printf("yes");
            } else {
                printf("no");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("yes");
}

// Nested enum as a struct member type (common in git: wt-status DIR_*, am MERGE_*, etc.).
// Enumerators must be in scope after the struct definition.
TEST(Compiler, enumAnonymousInStructMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            enum { A = 1, B = 2 } flags;
        };
        int main() {
            struct S s;
            s.flags = A | B;
            printf("%d %d %d", A, B, s.flags);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, enumNamedInStructMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            enum Color { RED, GREEN, BLUE } c;
        };
        int main() {
            struct S s;
            s.c = GREEN;
            printf("%d %d", RED, s.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

// Flag-style enumerators with shifts, as in wt-status.h / am.c.
TEST(Compiler, enumBitflagsInStructMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct Worktree {
            enum {
                DIR_SHOW_IGNORED = 1 << 0,
                DIR_SHOW_OTHER = 1 << 1,
                DIR_HIDE_EMPTY = 1 << 2
            } flags;
        };
        int main() {
            int f;
            f = DIR_SHOW_IGNORED | DIR_HIDE_EMPTY;
            printf("%d %d %d %d", DIR_SHOW_IGNORED, DIR_SHOW_OTHER, DIR_HIDE_EMPTY, f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 4 5");
}

// Enum constants through expression.
TEST(Compiler, enumConstantsInExpression) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        enum E { A = 10, B = 20, C = 12 };
        int main() {
            printf("%d", A + B + C);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Enumerator redefinition.
TEST(Compiler, enumeratorRedefinitionIsError) {
    SourceProgram program{R"prg(
        enum E { A = 1, A = 2 };
        int main() { return 0; }
    )prg"};
    program.compile();
    program.assertCompilationErrors("error: redefinition of enumerator");
    EXPECT_THAT(program.getCompilationErrors(), Not(HasSubstr("parsing failed")));
    EXPECT_THAT(program.getCompilationErrors(), Not(HasSubstr("Error: redefinition")));
}

TEST(Compiler, enumeratorBareRedefinitionIsError) {
    SourceProgram program{R"prg(
        enum { A, A };
        int main() { return 0; }
    )prg"};
    program.compile();
    program.assertCompilationErrors("error: redefinition of enumerator");
    EXPECT_THAT(program.getCompilationErrors(), Not(HasSubstr("parsing failed")));
    EXPECT_THAT(program.getCompilationErrors(), Not(HasSubstr("Error: redefinition")));
}

TEST(Compiler, enumeratorInitializerNotConstantIsError) {
    SourceProgram program{R"prg(
        int f(void);
        enum { A = f() };
        int main() { return 0; }
    )prg"};
    program.compile();
    program.assertCompilationErrors("enumerator initializer is not a constant expression");
    EXPECT_THAT(program.getCompilationErrors(), Not(HasSubstr("parsing failed")));
    EXPECT_THAT(program.getCompilationErrors(), Not(HasSubstr("Error: enumerator")));
}

TEST(Compiler, enumSameValueRedefinitionIsError) {
    SourceProgram program{R"prg(
        enum E { A = 1, A = 1 };
        int main() { return 0; }
    )prg"};
    program.compile();
    program.assertCompilationErrors("redefinition of enumerator");
}

TEST(Compiler, enumBitwiseNotInEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            enum { ALL = ~0 };
            printf("%d", ALL == -1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, enumRedefinitionInStructMemberIsError) {
    SourceProgram program{R"prg(
        struct S { enum { A = 1, A = 2 } flags; };
        int main() { return 0; }
    )prg"};
    program.compile();
    program.assertCompilationErrors("redefinition of enumerator");
}

TEST(Compiler, enumFileScopeObjectRedeclIsError) {
    SourceProgram program{R"prg(
        enum { A = 1 };
        int A;
        int main() { return 0; }
    )prg"};
    program.compile();
    program.assertCompilationErrors("redefinition of enumerator");
}

TEST(Compiler, enumeratorInBlockDoesNotLeakAfterBrace) {
    SourceProgram program{R"prg(
        int main() {
            {
                enum { A = 7 };
            }
            return A;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("symbol `A` is not defined");
}

TEST(Compiler, enumeratorOuterRestoredAfterInnerEnum) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 1 };
        int main() {
            {
                enum { A = 2 };
            }
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, enumeratorInnerHidesOuter) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 1 };
        int main() {
            enum { A = 2 };
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, enumObjectShadowHidesEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        enum { A = 1 };
        int main() {
            int A;
            A = 5;
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}


// GCC/SysV: enumerator range selects the enum's underlying integer type.
TEST(Compiler, enumLargeConstantSizeofIsEight) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum E { A = 0x100000000L };
        int main(void) {
            printf("%d", (int)sizeof(enum E));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, enumLargeConstantValuePreserved) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum E { A = 0x100000000L };
        int main(void) {
            printf("%ld", (long)A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967296");
}

TEST(Compiler, enumLargeConstantAutoIncrement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum E { A = 0x100000000L, B };
        int main(void) {
            printf("%ld %ld", (long)A, (long)B);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967296 4294967297");
}

TEST(Compiler, enumLargeConstantVariableRoundTrip) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum E { A = 0x100000000L };
        int main(void) {
            enum E e;
            e = A;
            printf("%d %ld", (int)sizeof(e), (long)e);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 4294967296");
}

TEST(Compiler, enumMaxLongConstantSizeofAndValue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum E { A = 9223372036854775807L };
        int main(void) {
            printf("%d %ld", (int)sizeof(enum E), (long)A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 9223372036854775807");
}

// Values in (INT_MAX, UINT_MAX] keep sizeof 4 as unsigned int (GCC/SysV).
TEST(Compiler, enumUnsignedIntUnderlyingSizeofAndValue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum U { X = 0x80000000u };
        int main(void) {
            enum U u;
            u = X;
            printf("%d %u %u", (int)sizeof(enum U), (unsigned)X, (unsigned)u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 2147483648 2147483648");
}

} // namespace
