#include "TestFixtures.h"

namespace {

TEST(Compiler, localArrayReadWrite) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
TEST(Compiler, multiDimensionalArrayAccess) {
    SourceProgram program{R"prg(
        int main() {
            int a[2][3];
            a[0][0] = 1;
            a[0][1] = 2;
            a[0][2] = 3;
            a[1][0] = 4;
            a[1][1] = 5;
            a[1][2] = 6;
            printf("%d %d %d %d", a[0][0], a[0][2], a[1][0], a[1][2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3 4 6");
}

TEST(Compiler, multiDimensionalCharArrayIndex) {
    SourceProgram program{R"prg(
        static char topath[4][8];
        int main() {
            int i;
            topath[1][0] = 65;
            topath[2][0] = 66;
            i = 1;
            if (topath[i][0]) {
                printf("%c", topath[i][0]);
            }
            i = 2;
            printf("%c", topath[i][0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("AB");
}

// Array-to-pointer decay for unary *: *arr is arr[0] (C 6.3.2.1 / 6.5.3.2).
TEST(Compiler, unaryDerefOnArray) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
TEST(Compiler, memberCharArrayPlusIndex) {
    SourceProgram program{R"prg(
        struct Ctx {
            unsigned long total;
            unsigned char buffer[64];
        };
        int main() {
            struct Ctx c;
            int left;
            unsigned char *p;
            c.buffer[0] = 65;
            c.buffer[1] = 66;
            c.buffer[2] = 67;
            c.buffer[3] = 68;
            left = 2;
            p = c.buffer + left;
            printf("%c%c", p[0], p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("CD");
}

// Direct call arg: memcpy(ctx->buffer + left, src, n) must pass the pointer
// value, not &stack_temp of a phantom array result. Without pointer-arithmetic
// recognition for member arrays (result is already decayed pointer, type stays
// array for sizeof), + yields an array temporary and call-arg decay takes its
// address - SHA1DCUpdate never writes the real buffer (wrong empty hash).
TEST(Compiler, memcpyIntoMemberArrayPlusOffset) {
    SourceProgram program{R"prg(
        void *memcpy(void *d, const void *s, unsigned long n);
        struct Ctx {
            unsigned long total;
            unsigned char buffer[64];
            int flag;
        };
        void update(struct Ctx *ctx, const char *buf, unsigned len) {
            unsigned left;
            left = (unsigned)(ctx->total & 63);
            if (len > 0) {
                ctx->total = ctx->total + (unsigned long)len;
                memcpy(ctx->buffer + left, buf, (unsigned long)len);
            }
        }
        int main() {
            struct Ctx c;
            char ch;
            int i;
            for (i = 0; i < 64; i = i + 1) {
                c.buffer[i] = 0xAA;
            }
            c.total = 0;
            ch = (char)0x80;
            update(&c, &ch, 1);
            printf("%d %d", (int)c.buffer[0], (int)c.total);
            c.total = 3;
            c.buffer[0] = 1;
            c.buffer[1] = 2;
            c.buffer[2] = 3;
            update(&c, "XY", 2);
            printf(" %d %d %d %d %d %d", (int)c.buffer[0], (int)c.buffer[1],
                    (int)c.buffer[2], (int)c.buffer[3], (int)c.buffer[4],
                    (int)c.total);
            return 0;
        }
    )prg"};
    program.compile();
    // 128 1 from first update; then 1 2 3 'X' 'Y' total 5
    program.runAndExpect("128 1 1 2 3 88 89 5");
}

// End-to-end: partial fill of struct char buffer via pointer arithmetic + cast
// of a static padding array (SHA1DCFinal shape).
TEST(Compiler, sha1PaddingCastAndBufferOffset) {
    SourceProgram program{R"prg(
        static unsigned char pad[64];
        struct Ctx {
            unsigned long total;
            unsigned char buffer[64];
        };
        void fill(struct Ctx *ctx, const char *buf, int len) {
            int left;
            left = (int)(ctx->total & 63);
            if (len > 0) {
                int i;
                unsigned char *dst;
                dst = ctx->buffer + left;
                for (i = 0; i < len; i = i + 1) {
                    dst[i] = (unsigned char)buf[i];
                }
                ctx->total = ctx->total + (unsigned long)len;
            }
        }
        int main() {
            struct Ctx c;
            int i;
            c.total = 0;
            for (i = 0; i < 64; i = i + 1) {
                c.buffer[i] = 0;
            }
            pad[0] = 128;
            pad[1] = 0;
            pad[2] = 0;
            fill(&c, "hi", 2);
            fill(&c, (const char *)(pad), 3);
            printf("%d %d %d %d %d", (int)c.buffer[0], (int)c.buffer[1],
                    (int)c.buffer[2], (int)c.buffer[3], (int)c.buffer[4]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 128 0 0");
}

// git t/unit-tests/u-hashmap.c: char seen[ARRAY_SIZE(key_val)] = { 0 };
// ARRAY_SIZE is sizeof(x)/sizeof((x)[0]). The bound must fold to a constant
// array size; if left non-constant the declarator decays to char* and
// key_val_contains(map, seen, ...) loads garbage instead of &seen[0].
TEST(Compiler, arraySizeofBoundFoldsLocalCharArray) {
    SourceProgram program{R"prg(
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
    program.compileWithPreprocess();
    program.runAndExpect("4 1 0 1 0");
}

// Same without the macro (sizeof division as array bound).
TEST(Compiler, arraySizeofDivisionBoundAsArrayNotPointer) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

} // namespace
