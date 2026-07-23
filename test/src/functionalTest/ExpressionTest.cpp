#include "TestFixtures.h"

namespace {

TEST(Compiler, unaryPlusPreserves) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d", +a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0", "0");
    program.runAndExpect("5", "5");
    program.runAndExpect("-2", "-2");
}

// C usual arithmetic conversions: unsigned int vs signed -1 must compare equal
// after converting -1 to unsigned (UINT_MAX). Without zero-extending the signed
// side to the common unsigned width, 0x00000000ffffffff != 0xffffffffffffffff
// (git create_tag_options.sign = -1; if (opt.sign == -1) ... breaks git tag).
TEST(Compiler, unsignedIntEqualsSignedMinusOne) {
    SourceProgram program{R"prg(
        int main() {
            unsigned int sign;
            int m1;
            sign = -1;
            m1 = -1;
            if (sign == -1) {
                printf("lit ");
            } else {
                printf("no_lit ");
            }
            if (sign == m1) {
                printf("var ");
            } else {
                printf("no_var ");
            }
            if (sign == (unsigned)-1) {
                printf("cast");
            } else {
                printf("no_cast");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("lit var cast");
}

// Same pattern as git cmd_tag: unsigned field set to -1, then branch.
TEST(Compiler, unsignedSignSentinelEqualsMinusOne) {
    SourceProgram program{R"prg(
        struct Opt {
            unsigned int sign;
        };
        int main() {
            struct Opt opt;
            int cmdmode;
            opt.sign = -1;
            cmdmode = 108;
            if (opt.sign == -1) {
                opt.sign = cmdmode ? 0 : 1;
            }
            printf("%u", opt.sign);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, chainedAssignment) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            int c;
            c = 3;
            a = b = c;
            printf("%d %d %d", a, b, c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 3 3");
}

TEST(Compiler, commaOperatorDiscardsLeft) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            b = 1;
            a = (b = 2, 3);
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 2");
}

// git DUP_ARRAY / ALLOC_ARRAY: copy_array((p = xmalloc(n), p), src, ...).
// Assignment through a pointer lvalue must yield the NEW value as the
// expression result, not the value loaded before the store. Otherwise the
// first arg is the pre-assignment pointer (often NULL) and the new block
// is left uninitialized (git copy_pathspec SEGV / garbage attr_match_nr).
TEST(Compiler, assignThroughPointerYieldsNewValueAsArg) {
    SourceProgram program{R"prg(
        void *malloc(unsigned long);
        void fill(int *dst, int *src, int n) {
            int i;
            for (i = 0; i < n; i = i + 1) {
                dst[i] = src[i];
            }
        }
        int main() {
            int *p;
            int src[3];
            src[0] = 10;
            src[1] = 20;
            src[2] = 30;
            p = 0;
            fill((p = (int *)malloc(3 * sizeof(int)), p), src, 3);
            if (p == 0) return 1;
            printf("%d %d %d", p[0], p[1], p[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 20 30");
}

// Same for struct member pointer lvalue (git d->attr_match = xmalloc(...)).
TEST(Compiler, assignThroughMemberPointerYieldsNewValueAsArg) {
    SourceProgram program{R"prg(
        void *malloc(unsigned long);
        struct Holder {
            int *p;
        };
        void fill(int *dst, int *src, int n) {
            int i;
            for (i = 0; i < n; i = i + 1) {
                dst[i] = src[i];
            }
        }
        int main() {
            struct Holder h;
            int src[2];
            src[0] = 7;
            src[1] = 8;
            h.p = 0;
            fill((h.p = (int *)malloc(2 * sizeof(int)), h.p), src, 2);
            if (h.p == 0) return 1;
            printf("%d %d", h.p[0], h.p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8");
}

TEST(Compiler, logicalAndSkipsRight) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            0 && ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, logicalOrSkipsRight) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            1 || ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, logicalAndEvaluatesRight) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            1 && ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, logicalOrEvaluatesRight) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            0 || ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, doubleNotOnComparison) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            scanf("%ld %ld", &a, &b);
            printf("%d", !!(a == b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 0", "0");
}

TEST(Compiler, pointerEquality) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            a = 1;
            b = 2;
            printf("%d %d", &a == &a, &a == &b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

// Hex literals ending in f must not lose digits to a float-suffix strip (0xff).
TEST(Compiler, hexLiteralEndingInF) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d %d %d", (int)0xff, (int)0xf, (int)0x10);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("255 15 16");
}

// C integer suffixes (u/l/ul/ull) must fold to assembler-safe immediates (no "ul" for NASM).
TEST(Compiler, hexLiteralWithIntegerSuffixes) {
    SourceProgram program{R"prg(
        int main() {
            unsigned long a;
            unsigned long b;
            a = 0xfful;
            b = 0x100ull;
            printf("%d %d", (int)a, (int)b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("255 256");
}

// Usual arithmetic conversions: int + unsigned long must yield unsigned long
// (not int). Otherwise high bits of pointers/size_t are truncated (git cbtree:
// (struct cb_node *)(1 + (uintptr_t)node) stores a sign-extended 32-bit value).
TEST(Compiler, intPlusUnsignedLongPreservesHighBits) {
    SourceProgram program{R"prg(
        int main() {
            unsigned long p;
            unsigned long t1;
            unsigned long t2;
            p = 0x7fff12345678UL;
            t1 = p + 1;
            t2 = 1 + p;
            printf("%d %d", (int)(t1 == 0x7fff12345679UL), (int)(t2 == 0x7fff12345679UL));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

// Crit-bit tree style: tag a pointer in the low bit via 1+(uintptr_t)p, then recover.
TEST(Compiler, uintptrTaggedPointerAddOne) {
    SourceProgram program{R"prg(
        int main() {
            char buf[8];
            char *node;
            unsigned long tagged;
            char *recovered;
            node = buf;
            tagged = 1 + (unsigned long)node;
            recovered = (char *)(tagged - 1);
            printf("%d %d", (int)(tagged & 1), (int)(recovered == node));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

} // namespace
