#include "TestFixtures.h"

namespace {

// Struct member arrays are sized at parse time (no semantic visit of the declarator first).
TEST(Compiler, structMemberArrayAccess) {
    SourceProgram program{R"prg(
        struct S {
            int vals[3];
        };
        int main() {
            struct S s;
            s.vals[0] = 1;
            s.vals[1] = 2;
            s.vals[2] = 3;
            printf("%d", s.vals[0] + s.vals[1] + s.vals[2]);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("6");
}

// Size expressions with sizeof must fold when building struct member types.
TEST(Compiler, structMemberArrayWithSizeof) {
    SourceProgram program{R"prg(
        struct S {
            unsigned long bits[(16 / sizeof(unsigned long))];
        };
        int main() {
            struct S s;
            s.bits[0] = 7;
            printf("%d", (int)s.bits[0]);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("7");
}

// Parameter array declarators decay to pointers (C).
TEST(Compiler, parameterArrayDecaysToPointer) {
    SourceProgram program{R"prg(
        int sum2(int a[2]) {
            return a[0] + a[1];
        }
        int main() {
            int v[2];
            v[0] = 10;
            v[1] = 32;
            printf("%d", sum2(v));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("42");
}

// Abstract parameter array (no name): int f(int [2]);
TEST(Compiler, abstractParameterArray) {
    SourceProgram program{R"prg(
        int sum2(int [2]);
        int sum2(int a[2]) {
            return a[0] + a[1];
        }
        int main() {
            int v[2];
            v[0] = 20;
            v[1] = 22;
            printf("%d", sum2(v));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("42");
}

// Zero-length array members appear in system headers / flexible array style.
TEST(Compiler, zeroLengthArrayMember) {
    SourceProgram program{R"prg(
        struct S {
            int n;
            char data[0];
        };
        int main() {
            struct S s;
            s.n = 5;
            printf("%d", s.n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("5");
}

// Incomplete parameter array T name[] decays to pointer (C; common in system headers).
TEST(Compiler, incompleteParameterArrayDecaysToPointer) {
    SourceProgram program{R"prg(
        int sum_n(int a[], int n) {
            int s;
            int i;
            s = 0;
            for (i = 0; i < n; i = i + 1) {
                s = s + a[i];
            }
            return s;
        }
        int main() {
            int v[3];
            v[0] = 10;
            v[1] = 20;
            v[2] = 12;
            printf("%d", sum_n(v, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("42");
}

// Abstract incomplete parameter array: char *const argv[] in prototypes.
TEST(Compiler, abstractIncompleteParameterArray) {
    SourceProgram program{R"prg(
        int first(int []);
        int first(int a[]) {
            return a[0];
        }
        int main() {
            int v[1];
            v[0] = 42;
            printf("%d", first(v));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("42");
}

// VLA-style parameter bound uses a prior parameter name; still decays to pointer.
// Appears in glibc headers (e.g. regexec ... regmatch_t __pmatch[__nmatch]).
// Avoid NASM reserved macro name `at` for the function.
TEST(Compiler, vlaStyleParameterArrayDecaysToPointer) {
    SourceProgram program{R"prg(
        int elem_at(int n, int a[n], int i) {
            return a[i];
        }
        int main() {
            int v[3];
            v[0] = 7;
            v[1] = 8;
            v[2] = 9;
            printf("%d", elem_at(3, v, 1));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("8");
}

// Multi-level incomplete arrays in prototypes (e.g. char *const argv[]).
TEST(Compiler, incompletePointerParameterArray) {
    SourceProgram program{R"prg(
        int first_len(char *const argv[]) {
            int n;
            n = 0;
            while (argv[n]) {
                n = n + 1;
            }
            return n;
        }
        int main() {
            char *argv[3];
            argv[0] = "a";
            argv[1] = "b";
            argv[2] = 0;
            printf("%d", first_len(argv));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("2");
}

// Multi-dimensional abstract array parameter in a prototype must match the
// definition after array-to-pointer decay (git sha1dc: uint32_t[80][5]).
TEST(Compiler, multiDimAbstractArrayParameterPrototype) {
    SourceProgram program{R"prg(
        void fill(int[2][3]);
        void fill(int a[2][3]) {
            a[0][0] = 1;
            a[0][1] = 2;
            a[0][2] = 3;
            a[1][0] = 4;
            a[1][1] = 5;
            a[1][2] = 6;
        }
        int main() {
            int m[2][3];
            fill(m);
            printf("%d %d %d %d", m[0][0], m[0][2], m[1][0], m[1][2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3 4 6");
}

// git FLEX_ALLOC_MEM: memcpy((void *)(x)->flexname, buf, len).
// Array member as rvalue after cast must yield the field address, not an
// uninitialized temporary (MemberAccess skips load for arrays).
TEST(Compiler, castArrayMemberToPointerForMemcpy) {
    SourceProgram program{R"prg(
        void *calloc(unsigned long n, unsigned long sz);
        void *memcpy(void *d, const void *s, unsigned long n);
        struct E {
            int len;
            char data[1];
        };
        struct E *make(const char *buf, unsigned long len) {
            struct E *e;
            e = calloc(1, 16 + len);
            memcpy((void *)e->data, buf, len);
            e->len = (int)len;
            return e;
        }
        int main() {
            struct E *e;
            e = make("hi", 2);
            printf("%d %c%c", e->len, e->data[0], e->data[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 hi");
}

// Same decay without memcpy: cast of array member used as pointer value.
TEST(Compiler, castArrayMemberAssignedToPointer) {
    SourceProgram program{R"prg(
        struct S {
            int n;
            char buf[4];
        };
        int main() {
            struct S s;
            char *p;
            s.buf[0] = 'a';
            s.buf[1] = 'b';
            s.buf[2] = 0;
            p = (char *)s.buf;
            printf("%s %d", p, p == s.buf);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ab 1");
}

// Flexible-style incomplete array member through pointer (git flex array).
TEST(Compiler, incompleteArrayMemberCastMemcpy) {
    SourceProgram program{R"prg(
        void *calloc(unsigned long n, unsigned long sz);
        void *memcpy(void *d, const void *s, unsigned long n);
        struct E {
            int len;
            char data[];
        };
        int main() {
            struct E *e;
            e = calloc(1, 16 + 3);
            memcpy((void *)e->data, "xyz", 3);
            e->len = 3;
            printf("%d %c%c%c", e->len, e->data[0], e->data[1], e->data[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 xyz");
}


} // namespace
