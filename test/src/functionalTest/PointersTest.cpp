#include "TestFixtures.h"

#include <fstream>

namespace {

TEST(Compiler, writesThroughPointerArgumentBeforeAnyCall) {
    SourceProgram program{R"prg(
        int f(int* v) {
            *v = *v + 1;
            printf("%d\n", *v);
            return 0;
        }

        int main() {
            int a = 5;
            f(&a);
            printf("%d\n", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6\n6\n");
}

TEST(Compiler, compilesSwapProgram) {
    SourceProgram program{R"prg(
        int swap(int *x, int *y) {
            int temp;
            printf("%d\n%d\n", *x, *y);
            temp = *x;
            *x = *y;
            *y = temp;
            printf("%d\n%d\n", *x, *y);
            return 0;
        }

        int main() {
            int a = 0;
            int b = 1;
            printf("%d\n%d\n", a, b);
            printf("%d\n%d\n", &a, &b);
            swap (&a, &b);
            printf("%d\n%d\n", a, b);
            printf("%d\n%d\n", &a, &b);
            return 0;
        }
    )prg"};
    program.compile();
    program.run();

    std::ifstream expectedOutputStream{program.getOutputFilePath()};
    std::string outputLine;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    std::string firstAddressBefore;
    expectedOutputStream >> firstAddressBefore;
    std::string secondAddressBefore;
    expectedOutputStream >> secondAddressBefore;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    std::string firstAddressAfter;
    expectedOutputStream >> firstAddressAfter;
    std::string secondAddressAfter;
    expectedOutputStream >> secondAddressAfter;
    EXPECT_THAT(firstAddressBefore, Not(Eq(secondAddressBefore)));
    EXPECT_THAT(firstAddressBefore, Eq(firstAddressAfter));
    EXPECT_THAT(secondAddressBefore, Eq(secondAddressAfter));
}

TEST(Compiler, pointerToPointer) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int* p;
            int** pp;
            a = 5;
            p = &a;
            pp = &p;
            printf("%d %d", **pp, *p);
            **pp = 9;
            printf(" %d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 5 9");
}

TEST(Compiler, pointerToPointerArgument) {
    SourceProgram program{R"prg(
        int set(int** pp, int v) {
            **pp = v;
            return 0;
        }

        int main() {
            int a;
            int* p;
            a = 1;
            p = &a;
            set(&p, 7);
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

// Pointer + integer scales by sizeof(pointee). char* scale 1; others match
// this compiler's word stride for sub-word scalars (git handle_options argv+1).
TEST(Compiler, pointerPlusIntegerScales) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = a;
            p = p + 1;
            printf("%d", *p);
            p = p + 1;
            printf(" %d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

// Compound p += n / p -= n must scale like p = p + n (C 6.5.16.2 / 6.5.6).
// git xdiff: xdf->changed += 1 then free(xdf->changed - 1). Without scaling
// free() gets an interior pointer and aborts with free(): invalid pointer.
TEST(Compiler, pointerCompoundAssignScalesByPointeeSize) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = a;
            p += 1;
            printf("%d", *p);
            p += 1;
            printf(" %d", *p);
            p -= 1;
            printf(" %d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30 20");
}

// git SWAP(arr[i], arr[j]): buffer is unsigned char tmp[sizeof(a)] where a is
// arr[i]. sizeof(arr[0]) must fold to a constant array size, not decay to pointer
// (otherwise memcpy dest is garbage and lookup_object SEGV).
TEST(Compiler, sizeofArrayElementAsArrayBound) {
    SourceProgram program{R"prg(
        void *memcpy(void *d, const void *s, unsigned long n);
        int main() {
            int *arr[4];
            int x;
            int y;
            int i;
            int first;
            x = 1;
            y = 2;
            i = 1;
            first = 0;
            arr[0] = &x;
            arr[1] = &y;
            {
                void *_swap_a_ptr;
                void *_swap_b_ptr;
                unsigned char _swap_buffer[sizeof(arr[0])];
                _swap_a_ptr = &arr[i];
                _swap_b_ptr = &arr[first];
                memcpy(_swap_buffer, _swap_a_ptr, sizeof(arr[0]));
                memcpy(_swap_a_ptr, _swap_b_ptr, sizeof(arr[0]));
                memcpy(_swap_b_ptr, _swap_buffer, sizeof(arr[0]));
            }
            printf("%d %d", *arr[0], *arr[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

// Minimal xdiff free pattern: calloc(n+2), advance by one element, free(base).
TEST(Compiler, pointerCompoundAssignThenFreeMinusOne) {
    SourceProgram program{R"prg(
        void *calloc(unsigned long n, unsigned long sz);
        void free(void *p);
        int main() {
            int *base;
            int *p;
            base = (int *)calloc(4, sizeof(int));
            if (!base) {
                printf("oom");
                return 1;
            }
            p = base;
            p += 1;
            p[0] = 7;
            free(p - 1);
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Pointer difference: (p - q) is in elements, not bytes (C 6.5.6).
// git prune_directory: dir->nr = dst - dir->entries (dir_entry**).
// Without scaling, one kept entry yields nr=8 and later null-deref.
TEST(Compiler, pointerDifferenceScalesByPointeeSize) {
    SourceProgram program{R"prg(
        int main() {
            int a[4];
            int *p;
            int *q;
            long n;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            p = a;
            q = a + 3;
            n = q - p;
            printf("%ld", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

// Same for pointer-to-pointer (git dir_entry **dst - dir->entries).
TEST(Compiler, pointerToPointerDifference) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            int y;
            int *arr[4];
            int **src;
            int **dst;
            long n;
            x = 10;
            y = 20;
            arr[0] = &x;
            arr[1] = &y;
            arr[2] = 0;
            arr[3] = 0;
            src = arr;
            dst = arr;
            dst = dst + 1;
            n = dst - src;
            if (n != 1) {
                printf("bad %ld", n);
                return 1;
            }
            printf("ok %d", **src);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok 10");
}

// char** arithmetic: argv + 1 advances one pointer slot (8 bytes).
// Minimal form of git handle_options / main argv walking.
TEST(Compiler, pointerToPointerPlusInteger) {
    SourceProgram program{R"prg(
        int handle(const char ***argv, int *argc) {
            while (*argc > 0) {
                const char *cmd;
                cmd = (*argv)[0];
                if (cmd[0] != '-')
                    break;
                if (cmd[1] == 'v') {
                    printf("v");
                    return 1;
                }
                (*argv) = (*argv) + 1;
                (*argc) = (*argc) - 1;
            }
            return 0;
        }
        int main() {
            const char *args[3];
            const char **p;
            int n;
            args[0] = "-x";
            args[1] = "hello";
            args[2] = 0;
            p = args;
            n = 2;
            handle(&p, &n);
            printf("%s %d", p[0], n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("hello 1");
}

// git container_of: nested (unsigned long)&((T*)0)->member cast chain then ->field.
TEST(Compiler, containerOfNestedCastArrow) {
    SourceProgram program{R"prg(
        struct H {
            int e;
        };
        struct S {
            struct H ent;
            int *es;
        };
        int main() {
            struct S s;
            struct H *eptr;
            int *a;
            int v;
            v = 42;
            s.es = &v;
            eptr = &s.ent;
            a = ((struct S *) ((char *)(eptr) - ((unsigned long)&((struct S *)0)->ent)))->es;
            printf("%d", *a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// git/kwset and system headers use trailing const in casts: (T const *),
// (unsigned char const *). Right-hand of right-recursive spec_qualifier_list
// may be only a qualifier (no TypeSpecifier); combine must not pop empty.
TEST(Compiler, castWithTrailingConstQualifier) {
    SourceProgram program{R"prg(
        struct K {
            int n;
        };
        int main() {
            unsigned char c;
            unsigned char const *cp;
            struct K k;
            struct K const *kp;
            c = 65;
            cp = (unsigned char const *)&c;
            k.n = 3;
            kp = (struct K const *)&k;
            printf("%d %d", (int)*cp, kp->n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65 3");
}

// git object-name.c:repo_find_unique_abbrev_r:
//   unsigned len; odb_find_abbrev_len(..., &len); hex[len] = 0;
// A 4-byte store through unsigned* leaves dirty high bits in the stack slot.
// Indexing/pointer+unsigned must zero-extend, not add the full qword.
TEST(Compiler, unsignedIndexAfterNarrowStoreThroughPointer) {
    SourceProgram program{R"prg(
        void write_len(unsigned *p) {
            *p = 5;
        }
        int main() {
            char hex[64];
            unsigned len;
            int i;
            unsigned long *poison;
            for (i = 0; i < 64; i = i + 1) {
                hex[i] = 120;
            }
            poison = (unsigned long *)&len;
            *poison = 0xffffffffffffffffUL;
            write_len(&len);
            hex[len] = 90;
            printf("%c%c%u", hex[5], hex[0], len);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("Zx5");
}

// &ptr[i] must form the address without loading *ptr[i]. Loading first SEGV's
// when the computed address is OOB - git index-pack read_v2_anomalous_offsets:
//   check_pack_index_ptr(p, &idx2[off * 2]);
// must die on a corrupt extended offset, not crash.
TEST(Compiler, addressOfPointerSubscriptNoLoad) {
    SourceProgram program{R"prg(
        int main() {
            unsigned buf[4];
            unsigned *idx2;
            unsigned off;
            unsigned long diff;
            const unsigned *p;
            buf[0] = 0;
            buf[1] = 0;
            buf[2] = 33;
            buf[3] = 0;
            idx2 = buf;
            off = 1;
            p = &idx2[off * 2];
            diff = (unsigned long)((const char *)p - (const char *)buf);
            printf("%lu %u", diff, *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 33");
}

// Forming &idx2[huge] must not dereference. Bounds check via pointer compare.
TEST(Compiler, addressOfOobPointerSubscriptNoSegv) {
    SourceProgram program{R"prg(
        int check(const void *base, const void *ptr, unsigned long size) {
            const char *p;
            const char *s;
            const char *e;
            p = (const char *)ptr;
            s = (const char *)base;
            e = s + size;
            if (p < s || p >= e - 8)
                return 1;
            return 0;
        }
        int main() {
            unsigned buf[4];
            unsigned *idx2;
            unsigned off;
            const unsigned *p;
            buf[0] = 0;
            buf[1] = 0;
            buf[2] = 0;
            buf[3] = 0;
            idx2 = buf;
            off = 0x7f000000u;
            p = &idx2[off * 2];
            printf("%d", check(buf, p, 16));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Same shape as git: check then conditional load of idx2[off*2].
TEST(Compiler, checkPackIndexPtrBeforeLoad) {
    SourceProgram program{R"prg(
        int check(const void *base, const void *ptr, unsigned long size) {
            const char *p;
            const char *s;
            const char *e;
            p = (const char *)ptr;
            s = (const char *)base;
            e = s + size;
            if (p < s || p >= e - 8)
                return 1;
            return 0;
        }
        int main() {
            unsigned buf[4];
            unsigned *idx2;
            unsigned off;
            buf[0] = 0;
            buf[1] = 0;
            buf[2] = 0;
            buf[3] = 0;
            idx2 = buf;
            off = 0xff000000u;
            if (!(off & 0x80000000u))
                return 1;
            off = off & 0x7fffffffu;
            if (check(buf, &idx2[off * 2], 16)) {
                printf("die");
                return 0;
            }
            if (idx2[off * 2])
                printf("nz");
            else
                printf("z");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("die");
}

} // namespace
