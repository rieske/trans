#include "TestFixtures.h"

namespace {

TEST(Compiler, sizeofTypeInt) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", sizeof(int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, sizeofTypeChar) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", sizeof(char));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, sizeofTypePointer) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", sizeof(int*));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, sizeofExpressionVariable) {
    SourceProgram program{R"prg(
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

TEST(Compiler, sizeofSizedArray) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
        int main() {
            printf("%d", sizeof(void));
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
    // sizeof on a function (not a pointer-to-function) is invalid; must not fold to 0.
    SourceProgram program{R"prg(
        int f() {
            return 1;
        }

        int main() {
            printf("%d", sizeof f);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("sizeof");
}

TEST(Compiler, sizeofPointerToFunction) {
    // Pointer-to-function is complete; must not be rejected as bare function.
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

} // namespace
