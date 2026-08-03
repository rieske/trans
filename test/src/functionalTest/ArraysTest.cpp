#include "TestFixtures.h"

#include <fstream>
#include <sstream>

namespace {

// Abstract array in a prototype (`char[20]` in glibc tmpnam) decays to pointer.
TEST(Compiler, abstractArrayParameterDecaysToPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int take(char[20]);

        int take(char s[20]) {
            return s[0];
        }

        int main() {
            char buf[20];
            buf[0] = 7;
            printf("%d", take(buf));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

// File-scope `char[20]` must not visit the bound in codegen (no procedure body).
TEST(Compiler, fileScopeAbstractArrayPrototypeCompiles) {
    SourceProgram program{R"prg(
        char tmpnam(char[20]);
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("");
}

TEST(Compiler, unsizedArrayCompletedFromBraceInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[] = { 1, 2, 3 };
            printf("%d %d %d %d", a[0], a[1], a[2], sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 12");
}

TEST(Compiler, unsizedCharArrayCompletedFromStringLiteral) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char s[] = "hi";
            printf("%d %d %d %d", s[0], s[1], s[2], sizeof s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3");
}

TEST(Compiler, charArrayStringInitIsNotInternedAsStringConstant) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char s[] = "xyzzy_no_pool_9f3a";
            printf("%d", sizeof s);
            return 0;
        }
    )prg"};
    program.addCompilerArg("-save-temps");
    program.compile();
    program.runAndExpect("19");
    EXPECT_THAT(program.readAssembly(), Not(HasSubstr("xyzzy_no_pool_9f3a")));
}

TEST(Compiler, unsizedCharArrayCompletedFromBracedStringLiteral) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char s[] = { "hi" };
            printf("%d %d %d %d", s[0], s[1], s[2], sizeof s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3");
}

TEST(Compiler, sizedCharArrayFromStringLiteral) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char s[3] = "hi";
            printf("%d %d %d %d", s[0], s[1], s[2], sizeof s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3");
}

TEST(Compiler, sizedCharArrayFromBracedStringLiteral) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char s[3] = { "hi" };
            printf("%d %d %d %d", s[0], s[1], s[2], sizeof s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3");
}

TEST(Compiler, sizedCharArrayFromStringTruncatesNul) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char s[2] = "hi";
            printf("%d %d %d", s[0], s[1], sizeof s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 2");
}

TEST(Compiler, sizedCharArrayFromStringPadsWithZeros) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char s[5] = "hi";
            printf("%d %d %d %d %d %d", s[0], s[1], s[2], s[3], s[4], sizeof s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 0 0 5");
}

TEST(Compiler, sizedCharArrayFromStringExcessIsError) {
    SourceProgram program{R"prg(
        int main() {
            char s[1] = "hi";
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements in array initializer");
}

// Global / DataWordSink path must use the same excess rule as FieldPlanSink.
// Must not also emit the generic "not a constant expression" after sink failure.
TEST(Compiler, globalSizedCharArrayFromStringExcessIsError) {
    SourceProgram program{R"prg(
        char g[1] = "hi";
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements in array initializer");
    EXPECT_EQ(program.getCompilationErrors().find("not a constant expression"),
            std::string::npos)
            << program.getCompilationErrors();
}

TEST(Compiler, unsizedIntArrayFromBracedStringIsOneElementNotIncomplete) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[] = { "hi" };
            printf("%d", sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

// Bare string does not count as a one-element list, so the array stays incomplete.
TEST(Compiler, unsizedIntArrayFromBareStringStaysIncomplete) {
    SourceProgram program{R"prg(
        int main() {
            int a[] = "hi";
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("incomplete type");
}

TEST(Compiler, unsizedArrayCompletedFromDesignatedInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[] = { [2] = 5 };
            printf("%d %d %d %d", a[0], a[1], a[2], sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 5 12");
}

TEST(Compiler, unsizedArrayNonConstantDesignatorIsError) {
    SourceProgram program{R"prg(
        int main() {
            int i = 1;
            int a[] = { [i] = 9 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("designated array index is not a constant expression");
}

TEST(Compiler, unsizedArrayMemberDesignatorIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a[] = { .x = 1 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("designated initializer member not found");
}

TEST(Compiler, unsizedArrayUndeclaredInitializerIsReportedOnce) {
    SourceProgram program{R"prg(
        int main() {
            int a[] = { nope };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("symbol `nope` is not defined");
    const std::string errors = program.getCompilationErrors();
    const std::string needle = "symbol `nope` is not defined";
    const auto first = errors.find(needle);
    ASSERT_NE(first, std::string::npos);
    EXPECT_EQ(errors.find(needle, first + needle.size()), std::string::npos) << errors;
}

TEST(Compiler, unsizedMultidimArrayCompletedFromNestedBraces) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[][2] = { { 1, 2 }, { 3, 4 } };
            printf("%d %d %d %d %d", a[0][0], a[0][1], a[1][0], a[1][1], sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4 16");
}

TEST(Compiler, unsizedArrayParameterDecaysToPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int take(char s[]) {
            return s[0];
        }

        int main() {
            char buf[2];
            buf[0] = 9;
            printf("%d", take(buf));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, paramVlaDecaysToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int take(int n, int a[n]) {
            return a[0];
        }
        int main() {
            int v[1];
            v[0] = 5;
            printf("%d", take(1, v));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, paramVlaAbstractPrototypeDecaysToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int take(int n, int [n]);
        int take(int n, int a[n]) {
            return a[0];
        }
        int main() {
            int v[1];
            v[0] = 7;
            printf("%d", take(1, v));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, localArrayReadWrite) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[3];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, arrayIndexExpression) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[5];
            int i;
            for (i = 0; i < 5; i = i + 1) {
                a[i] = i * 10;
            }
            printf("%d %d %d", a[0], a[2], a[4]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 20 40");
}

TEST(Compiler, arrayOfPointers) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int x;
            int y;
            int* p[2];
            x = 11;
            y = 22;
            p[0] = &x;
            p[1] = &y;
            printf("%d %d", *p[0], *p[1]);
            *p[1] = 33;
            printf(" %d", y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 22 33");
}

TEST(Compiler, pointerSubscript) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[3];
            int* p;
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            p = &a[0];
            printf("%d %d", p[1], p[2]);
            p[0] = 9;
            printf(" %d", a[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6 9");
}

TEST(Compiler, charArray) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char s[3];
            s[0] = 65;
            s[1] = 66;
            s[2] = 67;
            printf("%d %d %d", s[0], s[1], s[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65 66 67");
}

// Char array elements are packed (stride 1), so memcpy and s[i] agree.
TEST(Compiler, charArrayPackedMatchesMemcpy) {
    SourceProgram program{R"prg(#include <stdio.h>
        void *memcpy(void *d, const void *s, unsigned long n);
        int main() {
            char s[4];
            memcpy(s, "xyz", 3);
            s[3] = 0;
            printf("%s %d %d %d", s, s[0], s[1], s[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("xyz 120 121 122");
}

// char* into a packed byte buffer must use stride 1, not word stride.
// Required for git strbuf_setlen: sb->buf[len] = '\0'.
TEST(Compiler, charPointerPackedIndexRead) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char *p;
            p = "ABC";
            printf("%d %d %d", p[0], p[1], p[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65 66 67");
}

// Heap buffer write/read with packed char* (git strbuf / xrealloc path).
TEST(Compiler, charPointerHeapIndex) {
    SourceProgram program{R"prg(#include <stdio.h>
        char *malloc(int n);
        int main() {
            char *p;
            p = malloc(8);
            p[0] = 65;
            p[1] = 66;
            p[2] = 67;
            p[3] = 0;
            printf("%s", p);
            printf(" %d %d %d", p[0], p[1], p[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ABC 65 66 67");
}

// Multi-dimensional arrays: T a[N][M] is array of N arrays of M of T.
// Nested brackets must compose (used heavily in git, e.g. topath[4][...]).
TEST(Compiler, unaryDerefOnArray) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char reply[64];
            reply[0] = 121;
            if (*reply == 121) {
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

TEST(Compiler, unaryDerefOnStaticArray) {
    SourceProgram program{R"prg(#include <stdio.h>
        static char color[16];
        int main() {
            if (!*color) {
                color[0] = 88;
            }
            printf("%c %d", *color, !*color);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("X 0");
}

// Pointer to array (decay of multi-dim param / int (*p)[M]) must support p[i][j].
TEST(Compiler, pointerToArrayIndexing) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int m[2][3];
            int (*p)[3];
            p = m;
            p[0][0] = 1;
            p[0][2] = 3;
            p[1][0] = 4;
            p[1][2] = 6;
            printf("%d %d %d %d", p[0][0], p[0][2], p[1][0], p[1][2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3 4 6");
}

// Cast of a named array to pointer must pass &arr[0], not the first word of
// contents (git sha1dc: SHA1DCUpdate(ctx, (const char*)(sha1_padding), padn)
// where sha1_padding[0] == 0x80 would otherwise become a bogus pointer).
TEST(Compiler, castStaticArrayToPointerForCall) {
    SourceProgram program{R"prg(#include <stdio.h>
        static unsigned char pad[64];
        void take(const char *p, int n) {
            int i;
            for (i = 0; i < n; i = i + 1) {
                printf("%d", (int)(unsigned char)p[i]);
                if (i + 1 < n) {
                    printf(" ");
                }
            }
        }
        int main() {
            pad[0] = 128;
            pad[1] = 0;
            pad[2] = 7;
            take((const char *)(pad), 3);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("128 0 7");
}

// Same for a local array cast used as a pointer argument.
TEST(Compiler, castLocalArrayToPointerForCall) {
    SourceProgram program{R"prg(#include <stdio.h>
        int sum3(const char *p) {
            return (int)(unsigned char)p[0] + (int)(unsigned char)p[1]
                    + (int)(unsigned char)p[2];
        }
        int main() {
            unsigned char buf[8];
            buf[0] = 1;
            buf[1] = 2;
            buf[2] = 3;
            printf("%d", sum3((const char *)buf));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

// Member array + integer must scale by element size (1 for char), not by the
// whole array size (git sha1dc: memcpy(ctx->buffer + left, buf, len)).
TEST(Compiler, arraySizeofBoundFoldsLocalCharArray) {
    SourceProgram program{R"prg(#include <stdio.h>
        #define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
        static void mark(char *seen, int i) {
            seen[i] = 1;
        }
        int main() {
            const char *key_val[][2] = {
                { "key1", "value1" },
                { "key2", "value2" },
                { "foo", "value3" },
                { "bar", "value4" }
            };
            char seen[ARRAY_SIZE(key_val)] = { 0 };
            mark(seen, 0);
            mark(seen, 2);
            printf("%d %d %d %d %d", (int)ARRAY_SIZE(key_val),
                   (int)seen[0], (int)seen[1], (int)seen[2], (int)seen[3]);
            return 0;
        }
    )prg", "array_sizeof_bound"};
    program.compile();
    program.runAndExpect("4 1 0 1 0");
}

// Same without the macro (sizeof division as array bound).
TEST(Compiler, arraySizeofDivisionBoundAsArrayNotPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        static int sum(char *p, int n) {
            int s;
            int i;
            s = 0;
            for (i = 0; i < n; i = i + 1) {
                s = s + p[i];
            }
            return s;
        }
        int main() {
            const char *kv[][2] = { { "a", "b" }, { "c", "d" } };
            char seen[sizeof(kv) / sizeof(kv[0])] = { 0 };
            seen[0] = 3;
            seen[1] = 4;
            printf("%d", sum(seen, (int)(sizeof(kv) / sizeof(kv[0]))));
            return 0;
        }
    )prg", "array_sizeof_div_bound"};
    program.compile();
    program.runAndExpect("7");
}

// Local 2D char rows from string literals (git GREP_OPT_INIT colors[][COLOR_MAXLEN]).
// Without packing each string into its row, match highlighting colors are empty.
TEST(Compiler, localCharArray2DStringInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char colors[2][8] = { "\033[m", "\033[33m" };
            printf("%02x %02x %02x %02x %02x %s",
                (int)(unsigned char)colors[0][0],
                (int)(unsigned char)colors[0][1],
                (int)(unsigned char)colors[1][0],
                (int)(unsigned char)colors[1][1],
                (int)(unsigned char)colors[1][2],
                colors[1][0] ? "on" : "off");
            return 0;
        }
    )prg", "local_char2d_str"};
    program.compile();
    program.runAndExpect("1b 5b 1b 5b 33 on");
}

// Designated indices into local 2D char rows (git .colors = { [1] = GIT_COLOR_MAGENTA }).
TEST(Compiler, localCharArray2DDesignatedStringInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char colors[3][8] = { [0] = "", [1] = "\033[35m", [2] = "\033[1;31m" };
            printf("%d %02x %02x %02x %02x %02x %02x",
                colors[0][0] ? 1 : 0,
                (int)(unsigned char)colors[1][0],
                (int)(unsigned char)colors[1][1],
                (int)(unsigned char)colors[1][2],
                (int)(unsigned char)colors[2][0],
                (int)(unsigned char)colors[2][1],
                (int)(unsigned char)colors[2][2]);
            return 0;
        }
    )prg", "local_char2d_desig"};
    program.compile();
    // empty; ESC '[' '3'; ESC '[' '1'
    program.runAndExpect("0 1b 5b 33 1b 5b 31");
}

// git reftable_merged: struct T *refs[] = { r1, r2, r3 } where r1 is T[] must
// store &r1[0] (array decay), not memcpy the whole array into the pointer slot.
TEST(Compiler, pointerArrayInitDecaysArrayElements) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            char *name;
            long x;
            char pad[80];
        };
        int main() {
            struct S a[] = { { .name = (char *) "a", .x = 1 } };
            struct S b[] = { { .name = (char *) "b", .x = 2 } };
            struct S c[] = { { .name = (char *) "c", .x = 3 } };
            struct S *refs[] = { a, b, c };
            printf("%s %ld %s %ld %s %ld",
                   refs[0]->name, refs[0]->x,
                   refs[1]->name, refs[1]->x,
                   refs[2]->name, refs[2]->x);
            return 0;
        }
    )prg", "ptr_arr_decay"};
    program.compile();
    program.runAndExpect("a 1 b 2 c 3");
}

// Designated array index initializers.
TEST(Compiler, designatedArrayIndexInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[4] = { [2] = 30, [0] = 10, [1] = 2 };
            printf("%d", a[0] + a[1] + a[2] + a[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Incomplete local array completed by brace list.
TEST(Compiler, globalIncompleteArrayFromBrace) {
    SourceProgram program{R"prg(#include <stdio.h>
        int g[] = { 10, 20, 12 };
        int main() {
            printf("%d %d", (int)(sizeof(g)/sizeof(g[0])), g[0]+g[1]+g[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 42");
}

TEST(Compiler, arrayBraceInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[3] = { 1, 2, 3 };
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg", "array_brace_init"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, arrayBracePartialZeroFills) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[3] = { 7 };
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg", "array_brace_partial"};
    program.compile();
    program.runAndExpect("7 0 0");
}

TEST(Compiler, arrayOfStructBraceInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int x;
            int y;
        };
        int main() {
            struct S a[2] = { { 1, 2 }, { 3, 4 } };
            printf("%d %d %d %d", a[0].x, a[0].y, a[1].x, a[1].y);
            return 0;
        }
    )prg", "array_of_struct_brace"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}

// C: T *(a[N]) is the same as T *a[N] (array of pointers), not pointer-to-array.
TEST(Compiler, parenthesizedArrayOfPointersAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main(void) {
            const struct S *(oid[2]);
            struct S a;
            struct S b;
            a.x = 3;
            b.x = 4;
            oid[0] = &a;
            oid[1] = &b;
            printf("%d", oid[0]->x + oid[1]->x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

// Git grep shape: array of pointers, assign null and element.
TEST(Compiler, parenthesizedArrayOfPointersNullAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct E { int n; };
        int main(void) {
            struct E *(group[3]);
            group[0] = 0;
            group[1] = 0;
            group[2] = 0;
            printf("%d", group[0] == 0 && group[1] == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Pointer-to-array must stay distinct from array-of-pointers.
TEST(Compiler, pointerToArrayStillWorks) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int row[3];
            int (*p)[3];
            row[0] = 1;
            row[1] = 2;
            row[2] = 3;
            p = &row;
            printf("%d", (*p)[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

} // namespace
