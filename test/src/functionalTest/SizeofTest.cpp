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

TEST(Compiler, sizeofTypeLong) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", sizeof(long));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
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

TEST(Compiler, sizeofTypeVoidPointer) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", sizeof(void*));
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
TEST(Compiler, stringLiteralHexEscapesContentAndSizeof) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    program.compileWithPreprocess();
    program.runAndExpect("9 1 1");
}

// C translation phases: escape conversion (phase 5) then adjacent string
// concatenation (phase 6). Naive source join of "abc" "\x09" "def" yields
// "abc\x09def", where \x09def is one hex escape (0xef) and "def" is lost.
// git json-writer t0019 uses this shape: "abc" "\x09" "def".
TEST(Compiler, adjacentStringConcatHexEscapeDoesNotSwallowNextFragment) {
    SourceProgram program{R"prg(
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
    program.compileWithPreprocess();
    // a b c TAB d e f, sizeof 8 (7 payload + NUL)
    program.runAndExpect("61 62 63 09 64 65 66 8");
}

// Octal escapes in string literals (\0, \177).
TEST(Compiler, stringLiteralOctalEscapes) {
    SourceProgram program{R"prg(
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


} // namespace
