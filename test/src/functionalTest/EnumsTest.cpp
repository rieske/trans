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

TEST(Compiler, enumeratorBlockScopeObjectRedeclIsError) {
    SourceProgram program{R"prg(
        int main() {
            enum E { K };
            int K;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("redefinition of enumerator");
}

TEST(Compiler, enumeratorAfterBlockScopeObjectIsError) {
    SourceProgram program{R"prg(
        int main() {
            int K;
            enum E { K };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("redefinition of enumerator");
}

TEST(Compiler, enumeratorRedefinedAsFunctionDefinitionIsError) {
    SourceProgram program{R"prg(
        enum { A = 1 };
        int A(void) { return 0; }
        int main() { return 0; }
    )prg"};
    program.compile();
    program.assertCompilationErrors("redefinition of enumerator");
}

TEST(Compiler, blockScopeObjectShadowsFileScopeEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 5 };
        int main() {
            int A = 7;
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, parameterShadowsFileScopeEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 5 };
        int f(int A) { return A; }
        int main() {
            printf("%d %d", f(9), A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 5");
}

TEST(Compiler, innerEnumeratorShadowsOuterEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            enum { A = 5 };
            {
                enum { A = 1 };
                printf("%d", A);
            }
            printf(" %d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 5");
}

TEST(Compiler, blockEnumeratorShadowsFileScopeFunction) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int A(void) { return 0; }
        int main() {
            enum { A = 1 };
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, blockEnumeratorShadowsFileScopeObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int A = 7;
        int main() {
            enum { A = 1 };
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, innerEnumeratorShadowsBlockScopeObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int A = 7;
            {
                enum { A = 1 };
                printf("%d", A);
            }
            printf(" %d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 7");
}

TEST(Compiler, laterDeclaratorInSameDeclarationSeesTheObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { ERR = -1, OK = 0 };
        int main() {
            int ERR = 7, code = ERR;
            printf("%d %d", ERR, code);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 7");
}

TEST(Compiler, objectShadowingEnumeratorKeepsItsLvalue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 5 };
        int main() {
            int A = 0, *p = &A;
            *p = 3;
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, parameterBeforeARecordBraceStillShadowsAnEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 5 };
        int f(int A, struct S { int m; } *s) { (void)s; return A; }
        int main() {
            printf("%d", f(9, 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, prototypeParameterDoesNotHideAnEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            enum { N = 4 };
            int (*cb)(int N) = 0, k = N;
            (void)cb;
            printf("%d", k);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, forInitDeclaratorDoesNotHideAnEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            enum { A = 5 };
            {
                for (int A = 0; A < 3; A++) { }
                printf("%d", A);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, enumInPrototypeParameterListDoesNotShadowTheBlock) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int P = 7;
            {
                void f(enum { P = 3 } e);
                printf("%d", P);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, enumeratorInASizeofTypeNameShadowsTheFileScopeOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 9 };
        int main() {
            int n = (int)sizeof(enum { A = 1 });
            (void)n;
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, enumeratorInARecordMemberShadowsTheFileScopeOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 9 };
        int main() {
            struct T { enum { A = 1 } m; };
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, qualifiedEnumSpecifierStillDeclaresItsEnumerators) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int A = 55;
        int main() {
            (void)(const enum E2 { A = 3 })0;
            printf("%d", A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, sizeofAndGenericUseTheShadowingEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        long A = 7;
        int main() {
            enum { A = 1 };
            printf("%d %d", (int)sizeof A, _Generic(A, int: 4, long: 8, default: 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4");
}

TEST(Compiler, eachFunctionSeesItsOwnBlockEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int A = 7;
        int f(void) { enum { A = 1 }; return A; }
        int g(void) { enum { A = 2 }; return A; }
        int main() {
            printf("%d %d %d", f(), g(), A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 7");
}

TEST(Compiler, shadowingEnumeratorWorksAsACaseLabel) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int A = 7;
        int main() {
            enum { A = 2 };
            switch (2) { case A: printf("%d", 9); return 0; }
            return 1;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, arrayBoundUsesTheObjectShadowingAnEnumerator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 5 };
        int main() {
            int A = 3;
            int v[A];
            printf("%d", (int)sizeof v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
}

TEST(Compiler, bitFieldWidthFromAShadowedEnumeratorIsRejected) {
    SourceProgram program{R"prg(
        enum { A = 5 };
        int main(void) {
            int A = 3;
            struct S { unsigned b : A; } s;
            (void)s;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("bit-field width is not a constant");
}

TEST(Compiler, DISABLED_prototypeEnumeratorDoesNotOutliveItsDeclarator) {
    SourceProgram program{R"prg(
        void f(enum { R = 5 } e);
        int main(void) {
            return R;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("`R` is not defined");
}

TEST(Compiler, enumeratorDeclaredLaterInTheUnitIsNotVisible) {
    SourceProgram program{R"prg(
        int f(void) { return A; }
        enum { A = 5 };
        int main(void) { return f(); }
    )prg"};
    program.compile();
    program.assertCompilationErrors("`A` is not defined");
}

} // namespace
