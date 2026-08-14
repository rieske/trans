#include "TestFixtures.h"

namespace {

TEST(Compiler, sizeofTypeInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d", sizeof(int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, sizeofTypeChar) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d", sizeof(char));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, sizeofTypePointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d", sizeof(int*));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, sizeofTypePointerToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d", sizeof(int**));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, sizeofDoesNotEvaluateOperand) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            n = 0;
            printf("%d %d", (int)sizeof (++n), n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 0");
}

TEST(Compiler, sizeofExpressionVariable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int x;
            char c;
            int *p;
            printf("%d %d %d", sizeof x, sizeof c, sizeof p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 1 8");
}

// C: a string literal has type char[N] including NUL. sizeof is not pointer width.
TEST(Compiler, sizeofStringLiteralIsArrayLength) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)sizeof ":", (int)sizeof("ab"), (int)sizeof("a" "bc"));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 3 4");
}

TEST(Compiler, sizeofStringLiteralStillDecaysAsPointerValue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            const char *p;
            p = ":";
            printf("%d %s", (int)sizeof p, p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 :");
}

TEST(Compiler, sizeofSizedArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            printf("%d", sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
}

TEST(Compiler, sizedArrayDeclarationAccepted) {
    // Declaration alone must type-check; no element access required.
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[4];
            int b;
            b = 1;
            printf("%d", b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, nonConstantArraySizeIsSemanticError) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 3;
            int a[n];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is not a non-negative constant expression");
}

TEST(Compiler, sizeofArrayOfPointers) {
    // int *a[3] — array of pointers; also exercises pointer-qualified array elements.
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int *a[3];
            printf("%d", sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("24");
}

TEST(Compiler, sizeofAsArrayBound) {
    // sizeof expr folded as a constant bound — covers UnaryExpression::evaluateConstant for sizeof.
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[sizeof(int)];
            printf("%d", sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16");
}

TEST(Compiler, multidimensionalArraySizeof) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[2][3];
            printf("%d", sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("24");
}

TEST(Compiler, voidArrayIsSemanticError) {
    SourceProgram program{R"prg(
        int main() {
            void a[3];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array of incomplete type");
}

TEST(Compiler, voidArrayParameterIsSemanticError) {
    SourceProgram program{R"prg(
        int f(void a[3]) {
            return 0;
        }

        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array of incomplete type");
}

TEST(Compiler, arrayByteSizeOverflowIsSemanticError) {
    // element size 4 * 536870913 overflows signed 32-bit object size (INT_MAX).
    SourceProgram program{R"prg(
        int main() {
            int a[536870913];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is too large");
}

TEST(Compiler, arrayCountExceedsIntMaxIsSemanticError) {
    SourceProgram program{R"prg(
        int main() {
            char a[2147483648];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is too large");
}

TEST(Compiler, sizeofVoidTypeIsError) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d", sizeof(void));
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("sizeof");
}

TEST(Compiler, sizeofIncompleteStructTypeIsError) {
    // Incomplete and empty-complete both have getSize()==0; only incomplete is invalid.
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S;
        int main() {
            printf("%d", sizeof(struct S));
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("sizeof");
}

TEST(Compiler, negativeArraySizeIsSemanticError) {
    SourceProgram program{R"prg(
        int main() {
            int a[-1];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is not a non-negative constant expression");
}

TEST(Compiler, sizeofFunctionDesignatorIsError) {
    // ISO: sizeof on a function (not a pointer-to-function) is invalid; must not fold to 0.
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f() {
            return 1;
        }

        int main() {
            printf("%d", sizeof f);
            return 0;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("sizeof");
}

TEST(Compiler, sizeofFunctionDesignatorGnuIsOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(void) {
            return 1;
        }
        int main() {
            printf("%d", (int)sizeof f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// curl curlcheck_any_ptr: sizeof(fn) == sizeof(void *) is false (1 != 8).
TEST(Compiler, sizeofFunctionDesignatorIsNotPointerWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int cb(void) {
            return 0;
        }
        int main() {
            printf("%d", (int)(sizeof(cb) == sizeof(void *)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, sizeofPointerToFunction) {
    // Pointer-to-function is complete; must not be rejected as bare function.
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int (*f)();
            printf("%d", sizeof f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, sizeofArrayOfFunctionPointers) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int (*a[3])();
            printf("%d", sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("24");
}

TEST(Compiler, voidArrayParameterReportsSemanticErrorWithoutAbort) {
    // Recovery: FormalArgument diagnoses incomplete array element; FunctionDeclarator
    // must not rethrow when rebuilding argument types.
    SourceProgram program{R"prg(
        int f(void a[3]) {
            return 0;
        }

        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array of incomplete type");
    // Prefer the semantic-analysis path (context:line: error:), not an uncaught exception dump.
    program.assertCompilationErrors("error:");
}

TEST(Compiler, voidArrayPrototypeReportsSemanticErrorWithoutAbort) {
    SourceProgram program{R"prg(
        int f(void a[3]);
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array of incomplete type");
    program.assertCompilationErrors("error:");
}

TEST(Compiler, sizeofConstIntType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", (int)sizeof(const int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, sizeofVolatileIntType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", (int)sizeof(volatile int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

} // namespace
