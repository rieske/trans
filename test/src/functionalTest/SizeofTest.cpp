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

// sizeof(int [n]) is a runtime value (C99 VLA type, not an ICE).
TEST(Compiler, sizeofRuntimeArrayTypeIsElementCount) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 3;
            printf("%d", (int)sizeof(int [n]) / (int)sizeof(int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, sizeofRuntimeArrayTypeIsByteSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 3;
            printf("%d", (int)sizeof(int [n]));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
}

TEST(Compiler, sizeofPointerToRuntimeArrayIsPointerWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 3;
            printf("%d", (int)sizeof(int (*)[n]));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, sizeofRuntimeArrayTypeEvaluatesBoundAtUse) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int s;
            n = 2;
            s = (int)sizeof(int [n++]);
            printf("%d %d", s, n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 3");
}

TEST(Compiler, sizeofRuntimeArrayTypeIsNotIntegerConstant) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 3;
            static int a[sizeof(int [n])];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is not a non-negative constant expression");
}

TEST(Compiler, sizeofRuntimeArrayTypeNestedInnerConstant) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 3;
            printf("%d", (int)sizeof(int [n][2]));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("24");
}

// Unsigned compare of a cast must use the cast type, not a TypeCast special case
// in the binary folder. ((unsigned long)-1) > 0 is an ICE 1.
TEST(Compiler, unsignedCastCompareIsStaticInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int n = ((unsigned long)-1) > 0;
        int main() {
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// git parse-options.h OPT_UNSIGNED: static initializer uses
// .precision = sizeof(*v) and .value = v + BARF_UNLESS_UNSIGNED(*v),
// where BARF is sizeof(char[1 - 2*!(((__typeof__(var))-1) > 0)]) - 1.
// The bound is an ICE (unsigned cast compare), not a VLA.
TEST(Compiler, sizeofBuildAssertUnsignedIsStaticInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
#define BUILD_ASSERT_OR_ZERO(cond) (sizeof(char [1 - 2*!(cond)]) - 1)
#define BARF_UNLESS_UNSIGNED(var) BUILD_ASSERT_OR_ZERO(((__typeof__(var)) -1) > 0)
        static unsigned long batch_size;
        struct option {
            void *value;
            unsigned long precision;
        };
        static struct option opts[] = {
            {
                .value = &batch_size + BARF_UNLESS_UNSIGNED(*(&batch_size)),
                .precision = sizeof(*(&batch_size)),
            },
        };
        int main() {
            printf("%d %d", opts[0].value == (void *)&batch_size, (int)opts[0].precision);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 8");
}

// (int)-1 < 0 is true. Zero-extending the 32-bit pattern makes the bound 0.
TEST(Compiler, sizeofCharArraySignedCastNegativeIsOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int n = sizeof(char [((int)-1) < 0]);
        int main() {
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// git builtin/add.c OPT_BOOL / OPT_COUNTUP: file-scope struct option[]
// with .precision = sizeof(*v) for a file-scope int.
TEST(Compiler, sizeofFileScopeOptionPrecisionIsStaticInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct option {
            void *value;
            unsigned long precision;
        };
        static int show_only;
        static struct option opts[] = {
            {
                .value = &show_only,
                .precision = sizeof(*(&show_only)),
            },
        };
        int main() {
            printf("%d %d", opts[0].value == (void *)&show_only, (int)opts[0].precision);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 4");
}

// git builtin/add.c OPT_DIFF_UNIFIED / OPT_INTEGER: BARF_UNLESS_SIGNED of a
// struct member, plus sizeof of that member, in a file-scope option array.
TEST(Compiler, sizeofBuildAssertSignedMemberIsStaticInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
#define BUILD_ASSERT_OR_ZERO(cond) (sizeof(char [1 - 2*!(cond)]) - 1)
#define BARF_UNLESS_SIGNED(var) BUILD_ASSERT_OR_ZERO(((__typeof__(var)) -1) < 0)
        struct interactive_options {
            int context;
            int interhunkcontext;
        };
        static struct interactive_options interactive_opts = {
            .context = -1,
            .interhunkcontext = -1,
        };
        struct option {
            void *value;
            unsigned long precision;
        };
        static struct option opts[] = {
            {
                .value = &interactive_opts.context
                    + BARF_UNLESS_SIGNED(*(&interactive_opts.context)),
                .precision = sizeof(*(&interactive_opts.context)),
            },
        };
        int main() {
            printf("%d %d",
                opts[0].value == (void *)&interactive_opts.context,
                (int)opts[0].precision);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 4");
}

// sizeof of a VM type, not only the sizeof(type-name) spelling.
TEST(Compiler, sizeofDereferencedPointerToRuntimeArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int a[3];
            n = 3;
            printf("%d", (int)sizeof(*(int (*)[n])&a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
}

// typeof copies the VM type; the bound must live on Type, not on a nearby declarator.
TEST(Compiler, sizeofTypeofDereferencedPointerToRuntimeArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int a[3];
            n = 3;
            printf("%d", (int)sizeof(__typeof__(*(int (*)[n])&a)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
}

TEST(Compiler, sizeofRuntimeArrayTypeNestedOuterConstant) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 3;
            printf("%d", (int)sizeof(int [2][n]));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("24");
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

// Fuzz: *&fn is still a function designator; GNU sizeof is 1, not pointer width.
TEST(Compiler, sizeofDerefAddressOfFunctionDesignatorGnuIsOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int id(int x) { return x; }
        int main(void) {
            printf("%d %d", (int)sizeof(id), (int)sizeof(*&id));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, sizeofDerefFunctionPointerGnuIsOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int id(int x) { return x; }
        int main(void) {
            int (*fp)(int);
            fp = id;
            printf("%d", (int)sizeof(*fp));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, sizeofDerefAddressOfFunctionDesignatorIsError) {
    SourceProgram program{R"prg(
        int id(int x) { return x; }
        int main(void) {
            sizeof(*&id);
            return 0;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("sizeof");
}

TEST(Compiler, sizeofDerefFunctionPointerIsError) {
    SourceProgram program{R"prg(
        int id(int x) { return x; }
        int main(void) {
            int (*fp)(int);
            fp = id;
            sizeof(*fp);
            return 0;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("sizeof");
}

} // namespace
