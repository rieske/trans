#include "TestFixtures.h"

namespace {

TEST(Compiler, sizeofTypeInt) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", sizeof(int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, sizeofTypeChar) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", sizeof(char));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, sizeofTypeLong) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", sizeof(long));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, sizeofTypePointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", sizeof(int*));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, sizeofTypePointerToPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", sizeof(int**));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, sizeofDoesNotEvaluateOperand) {
    SourceProgram program{R"prg(#include <stdio.h>
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

TEST(Compiler, sizeofTypeVoidPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", sizeof(void*));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
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
    program.assertCompilationErrors("array size is negative");
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
    )prg", std::vector<std::string>{"-std=c"}};
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

TEST(Compiler, sizeofExpressionVariable) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int x;
            long y;
            char c;
            printf("%d %d %d", sizeof x, sizeof y, sizeof c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 8 1");
}

TEST(Compiler, sizeofExpressionParenthesized) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int x;
            printf("%d", sizeof(x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, sizeofArray) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[3];
            printf("%d", sizeof(a));
            return 0;
        }
    )prg"};
    program.compile();
    // System V: int array packs at natural size 4 * 3.
    program.runAndExpect("12");
}

// Int array inside a struct (SHA1_CTX shape): sizeof and field offsets must
// match C ABI so hash update code finds buffer[] after ihv[5].
TEST(Compiler, sizeofIntArrayInStructMatchesAbi) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Ctx {
            unsigned long total;
            unsigned int ihv[5];
            unsigned char buffer[64];
            int found;
        };
        int main() {
            struct Ctx c;
            char *base;
            char *buf;
            char *found;
            base = (char *)&c;
            buf = (char *)&c.buffer;
            found = (char *)&c.found;
            printf("%d %d %d", (int)sizeof(c), (int)(buf - base), (int)(found - base));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("96 28 92");
}

TEST(Compiler, sizeofInArithmetic) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int n;
            n = sizeof(int) + sizeof(char);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, sizeofStruct) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            struct S {
                int a;
                int b;
            };
            struct S s;
            printf("%d", sizeof(s));
            return 0;
        }
    )prg"};
    program.compile();
    // System V: two ints pack (offsets 0,4), size 8.
    program.runAndExpect("8");
}

// C allows sizeof(*p) in p's own initializer (declarator is complete before initializer).
TEST(Compiler, sizeofSelfReferentialPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int x;
            int y;
        };
        int main() {
            struct S *p;
            p = (struct S *)0;
            {
                struct S *wt = p;
                /* separate decl with sizeof self-ref: */
            }
            {
                int n;
                struct S *wt;
                n = sizeof(*wt);
                printf("%d", n);
            }
            return 0;
        }
    )prg"};
    program.compile();
    // System V: two ints -> 8
    program.runAndExpect("8");
}

// sizeof(*name) where name is the variable being initialized (git: xcalloc(1, sizeof(*wt))).
TEST(Compiler, sizeofInOwnInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int x;
            int y;
        };
        char pool[64];
        void *alloc(int n) {
            return pool;
        }
        int main() {
            struct S *wt = alloc(sizeof(*wt));
            wt->x = 3;
            wt->y = 4;
            printf("%d %d", wt->x, wt->y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

// sizeof("abc") is the array size including NUL (4), not pointer width.
// git t/unit-tests/u-ctype.c: ARRAY_SIZE(string) - 1 for TEST_CHAR_CLASS.
TEST(Compiler, sizeofStringLiteralIsArraySize) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d %d %d", (int)sizeof("abc"), (int)sizeof(" \n\r\t"),
                    (int)sizeof("0123456789"));
            return 0;
        }
    )prg", "sizeof_str_lit"};
    program.compile();
    // "abc" -> 4; " \n\r\t" -> 5; "0123456789" -> 11
    program.runAndExpect("4 5 11");
}

// Incomplete char array completed by string literal: sizeof includes the NUL.
// git wrapper.c git_mkdstemps_mode: static const char x_pattern[] = "XXXXXX";
// ARRAY_SIZE(x_pattern) - 1 must be 6, not -1 (sizeof was 0 before completion).
TEST(Compiler, sizeofIncompleteCharArrayFromStringLiteralGlobal) {
    SourceProgram program{R"prg(#include <stdio.h>
        static const char x_pattern[] = "XXXXXX";
        static const char letters[] = "ab";
        static const int num_x = (sizeof(x_pattern) / sizeof((x_pattern)[0])) - 1;
        static const int num_letters = (sizeof(letters) / sizeof((letters)[0])) - 1;
        int main() {
            printf("%d %d %d %d", (int)sizeof(x_pattern), num_x,
                    (int)sizeof(letters), num_letters);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 6 3 2");
}

TEST(Compiler, sizeofIncompleteCharArrayFromStringLiteralLocal) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char buf[] = "XXXXXX";
            char empty[] = "";
            printf("%d %d", (int)sizeof(buf), (int)sizeof(empty));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 1");
}

// Content of local char buf[] = "..." must be the string bytes, not the pointer
// to the literal. git clar sandbox: const char path_tail[] = "clar_tmp_XXXXXX";
// then strncpy(..., path_tail, ...) / mkdtemp on the result.
TEST(Compiler, localCharArrayStringInitializerContent) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            const char path_tail[] = "clar_tmp_XXXXXX";
            char buf[20];
            int i;
            i = 0;
            while (path_tail[i]) {
                buf[i] = path_tail[i];
                i = i + 1;
            }
            buf[i] = 0;
            printf("%s %d %d", buf, (int)sizeof(path_tail), path_tail[0]);
            return 0;
        }
    )prg", "local_char_arr_str"};
    program.compile();
    // sizeof includes NUL: 15 chars + 1 = 16; first char 'c' = 99
    program.runAndExpect("clar_tmp_XXXXXX 16 99");
}

// Function-scope static (same storage path as git_mkdstemps_mode locals).
TEST(Compiler, sizeofIncompleteCharArrayFromStringLiteralFunctionStatic) {
    SourceProgram program{R"prg(#include <stdio.h>
        int check(void) {
            static const char x_pattern[] = "XXXXXX";
            static const int num_x = (sizeof(x_pattern) / sizeof((x_pattern)[0])) - 1;
            return num_x;
        }
        int main() {
            printf("%d", check());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

// Function-scope static incomplete struct array completed by brace init.
// git unicode-width.h: static const struct interval double_width[] = { ... };
// inside git_wcwidth. ARRAY_SIZE must be the element count, not 0 (sizeof was
// incomplete because only the .data holder type was updated, not the local).
TEST(Compiler, sizeofIncompleteStructArrayBraceInitFunctionStatic) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct interval {
            int first;
            int last;
        };
        int array_size_of_table(void) {
            static const struct interval double_width[] = {
                { 0x1100, 0x115f },
                { 0x2329, 0x232a },
                { 0x2e80, 0x303e },
            };
            return (int)(sizeof(double_width) / sizeof(double_width[0]));
        }
        int bisearch_hit(int ucs) {
            static const struct interval table[] = {
                { 0x1100, 0x115f },
                { 0x2e80, 0x303e },
            };
            int min;
            int max;
            int mid;
            max = (int)(sizeof(table) / sizeof(table[0])) - 1;
            min = 0;
            if (ucs < table[0].first || ucs > table[max].last) {
                return 0;
            }
            while (max >= min) {
                mid = (min + max) / 2;
                if (ucs > table[mid].last) {
                    min = mid + 1;
                } else if (ucs < table[mid].first) {
                    max = mid - 1;
                } else {
                    return 1;
                }
            }
            return 0;
        }
        int main() {
            printf("%d %d %d", array_size_of_table(),
                    bisearch_hit(0x2f00), bisearch_hit(0x41));
            return 0;
        }
    )prg", "sizeof_static_struct_arr"};
    program.compile();
    // 3 elements; 0x2f00 is in [0x2e80,0x303e]; ASCII 'A' is not
    program.runAndExpect("3 1 0");
}

// Hex escapes in string literals: "\x00\x01" is two bytes (0, 1) plus NUL, not the
// characters x,0,0. git t/unit-tests/u-ctype.c ASCII/CNTRL tables use \xNN and
// classify via memchr(string, i, ARRAY_SIZE(string)-1).
// String and char escapes must share one product decoder (P0: no Scanner vs
// ConstantExpression vs util drift). Uppercase hex + control escapes end-to-end.
TEST(Compiler, stringAndCharEscapesUseSharedDecoder) {
    SourceProgram program{R"prg(#include <stdio.h>
        const char hex[] = "\x41\X42";
        const char ctl[] = "\a\b\f\v";
        int main() {
            printf("%c%c %d %d %d %d %d %d",
                hex[0], hex[1],
                (int)ctl[0], (int)ctl[1], (int)ctl[2], (int)ctl[3],
                (int)'\x41', (int)'\033');
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("AB 7 8 12 11 65 27");
}

TEST(Compiler, stringLiteralHexEscapesContentAndSizeof) {
    SourceProgram program{R"prg(#include <stdio.h>
        void *memchr(const void *s, int c, unsigned long n);
        int main() {
            const char *s = "\x00\x01\x02\x7f";
            int n = (int)sizeof("\x00\x01\x02\x7f");
            int f0 = memchr(s, 0, n - 1) ? 1 : 0;
            int f1 = memchr(s, 1, n - 1) ? 1 : 0;
            int f2 = memchr(s, 2, n - 1) ? 1 : 0;
            int f7f = memchr(s, 0x7f, n - 1) ? 1 : 0;
            printf("%d %d %d %d %d %d %d %d", n, f0, f1, f2, f7f,
                    (int)(unsigned char)s[0], (int)(unsigned char)s[1],
                    (int)(unsigned char)s[2]);
            return 0;
        }
    )prg", "str_hex_esc"};
    program.compile();
    // sizeof 5 (4 payload + NUL); all four bytes present; first three are 0,1,2
    program.runAndExpect("5 1 1 1 1 0 1 2");
}

// Adjacent string concatenation with hex escapes (ctype ASCII macro shape).
// Concat is handled in the scanner; use the preprocess path for realism.
TEST(Compiler, stringLiteralHexEscapesAdjacentConcat) {
    SourceProgram program{R"prg(#include <stdio.h>
        void *memchr(const void *s, int c, unsigned long n);
        int main() {
            const char *s = "\x00\x01\x02\x03" "\x04\x05\x06\x07";
            int n = (int)sizeof("\x00\x01\x02\x03" "\x04\x05\x06\x07");
            int f0 = memchr(s, 0, n - 1) ? 1 : 0;
            int f7 = memchr(s, 7, n - 1) ? 1 : 0;
            printf("%d %d %d", n, f0, f7);
            return 0;
        }
    )prg", "str_hex_adj"};
    program.compile();
    program.runAndExpect("9 1 1");
}

// C translation phases: escape conversion (phase 5) then adjacent string
// concatenation (phase 6). Naive source join of "abc" "\x09" "def" yields
// "abc\x09def", where \x09def is one hex escape (0xef) and "def" is lost.
// git json-writer t0019 uses this shape: "abc" "\x09" "def".
TEST(Compiler, adjacentStringConcatHexEscapeDoesNotSwallowNextFragment) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            const char *s = "abc" "\x09" "def";
            printf("%02x %02x %02x %02x %02x %02x %02x %d",
                    (int)(unsigned char)s[0], (int)(unsigned char)s[1],
                    (int)(unsigned char)s[2], (int)(unsigned char)s[3],
                    (int)(unsigned char)s[4], (int)(unsigned char)s[5],
                    (int)(unsigned char)s[6], (int)sizeof("abc" "\x09" "def"));
            return 0;
        }
    )prg", "str_hex_adj_boundary"};
    program.compile();
    // a b c TAB d e f, sizeof 8 (7 payload + NUL)
    program.runAndExpect("61 62 63 09 64 65 66 8");
}

// Octal escapes in string literals (\0, \177).
TEST(Compiler, stringLiteralOctalEscapes) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            const char *s = "\0\1\12\177";
            int n = (int)sizeof("\0\1\12\177");
            printf("%d %d %d %d %d", n,
                    (int)(unsigned char)s[0], (int)(unsigned char)s[1],
                    (int)(unsigned char)s[2], (int)(unsigned char)s[3]);
            return 0;
        }
    )prg", "str_oct_esc"};
    program.compile();
    // 4 payload bytes + NUL; values 0, 1, 10, 127
    program.runAndExpect("5 0 1 10 127");
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
    const std::string errors = program.getCompilationErrors();
    const std::string needle = "array of incomplete type";
    const auto first = errors.find(needle);
    ASSERT_NE(first, std::string::npos);
    EXPECT_EQ(errors.find(needle, first + needle.size()), std::string::npos) << errors;
}

TEST(Compiler, oversizedArrayParameterReportsRealTypeError) {
    SourceProgram program{R"prg(
        int f(int a[536870913]) {
            return 0;
        }

        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is too large");
}

TEST(Compiler, laterFunctionIsAnalyzedAfterEarlierSemanticError) {
    SourceProgram program{R"prg(
        int f() {
            return nope;
        }

        int g() {
            return also_missing;
        }

        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("symbol `nope` is not defined");
    program.assertCompilationErrors("symbol `also_missing` is not defined");
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
    )prg", std::vector<std::string>{"-std=c"}};
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
    )prg", std::vector<std::string>{"-std=c"}};
    program.compile();
    program.assertCompilationErrors("sizeof");
}

} // namespace
