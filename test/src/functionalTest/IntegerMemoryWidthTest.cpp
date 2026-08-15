#include "TestFixtures.h"

namespace {

// Named local, dirty 8 bytes at &obj, store through a typed pointer, then if / == 0.

TEST(Compiler, boolStoredThroughPointerThenIfUsesOnlyThatByte) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void set(bool *p, int v) {
            *p = v;
        }
        int main() {
            bool b;
            dirty8(&b);
            set(&b, 0);
            if (b) {
                printf("1");
            } else {
                printf("0");
            }
            set(&b, 1);
            if (b) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("01");
}

TEST(Compiler, boolStoredThroughPointerThenEqualsZero) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void set(bool *p, int v) {
            *p = v;
        }
        int main() {
            bool b;
            dirty8(&b);
            set(&b, 0);
            printf("%d ", b == 0);
            set(&b, 1);
            printf("%d", b != 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, charStoredThroughPointerThenIfIgnoresNeighborBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void setc(char *p, int v) {
            *p = v;
        }
        int main() {
            char c;
            dirty8(&c);
            setc(&c, 0);
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d ", c == 0);
            setc(&c, -1);
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 1");
}

TEST(Compiler, unsignedCharStoredThroughPointerThenIf) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void setuc(unsigned char *p, int v) {
            *p = v;
        }
        int main() {
            unsigned char c;
            dirty8(&c);
            setuc(&c, 0);
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            setuc(&c, 255);
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("01");
}

TEST(Compiler, shortStoredThroughPointerThenIfIgnoresNeighborBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void sets(short *p, int v) {
            *p = v;
        }
        int main() {
            short s;
            dirty8(&s);
            sets(&s, 0);
            if (s) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", s == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, manyLiveCharsThenDereferenceDoesNotCrashCompiler) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int sum(char *p) {
            char a0 = p[0];
            char a1 = p[1];
            char a2 = p[2];
            char a3 = p[3];
            char a4 = p[4];
            char a5 = p[5];
            char a6 = p[6];
            char a7 = p[7];
            char a8 = p[8];
            char a9 = p[9];
            char a10 = p[10];
            char a11 = p[11];
            char a12 = p[12];
            char a13 = p[13];
            char a14 = p[14];
            char a15 = p[15];
            char x = *p;
            return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7
                    + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15 + x;
        }
        int main() {
            char buf[16];
            int i;
            for (i = 0; i < 16; i++) {
                buf[i] = (char)(i + 1);
            }
            printf("%d", sum(buf));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("137");
}

TEST(Compiler, manyLiveShortsThenDereferenceDoesNotCrashCompiler) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int sum(short *p) {
            short a0 = p[0];
            short a1 = p[1];
            short a2 = p[2];
            short a3 = p[3];
            short a4 = p[4];
            short a5 = p[5];
            short a6 = p[6];
            short a7 = p[7];
            short a8 = p[8];
            short a9 = p[9];
            short a10 = p[10];
            short a11 = p[11];
            short a12 = p[12];
            short a13 = p[13];
            short a14 = p[14];
            short a15 = p[15];
            short x = *p;
            return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7
                    + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15 + x;
        }
        int main() {
            short buf[16];
            int i;
            for (i = 0; i < 16; i++) {
                buf[i] = (short)(i + 1);
            }
            printf("%d", sum(buf));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("137");
}

TEST(Compiler, intStoredThroughPointerThenIfIgnoresNeighborBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void seti(int *p, int v) {
            *p = v;
        }
        int main() {
            int n;
            dirty8(&n);
            seti(&n, 0);
            if (n) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", n == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, unsignedCharIncrementWrapsThenIfIsFalse) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            c = 255;
            c++;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", c == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, unsignedCharDecrementWrapsThenIfIsTrue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            c = 0;
            c--;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 255");
}

TEST(Compiler, globalBoolStoredThroughPointerThenIfUsesOnlyThatByte) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        bool g;
        void set(bool *p, int v) {
            *p = v;
        }
        int main() {
            set(&g, 0);
            if (g) {
                printf("1");
            } else {
                printf("0");
            }
            set(&g, 1);
            if (g) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("01");
}

TEST(Compiler, memoryResidentUnsignedCharArithUsesExtendedLoad) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char a;
            unsigned char b;
            a = 200;
            b = 13;
            printf("%d %d %d %d", (int)(a + b), (int)(a * b), (int)(a / b), (int)(a & b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("213 2600 15 8");
}

TEST(Compiler, unsignedShortIncrementWrapsThenIfIsFalse) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned short s;
            s = 65535;
            s++;
            if (s) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", s == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, unsignedCharParameterIncrementWraps) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned char step(unsigned char c) {
            c++;
            return c;
        }
        int main() {
            unsigned char z;
            z = step(255);
            if (z) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, intIncrementSignExtendsWhenUsedAsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = -2;
            n++;
            printf("%ld", (long)n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, adjacentIntFieldsIncrementDoesNotClobberNeighbors) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct triple {
            int a;
            int b;
            int c;
        };
        int main() {
            struct triple t;
            t.a = 1;
            t.b = 20;
            t.c = 300;
            t.a++;
            t.b++;
            printf("%d %d %d", t.a, t.b, t.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 21 300");
}

TEST(Compiler, adjacentStaticIntFieldsIncrementDoesNotClobberNeighbors) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct stats {
            int get_next;
            int set_next;
            int compare;
        };
        static struct stats stats;
        int main() {
            stats.get_next = 0;
            stats.set_next = 0;
            stats.compare = 0;
            stats.get_next++;
            stats.get_next++;
            stats.set_next++;
            printf("%d %d %d", stats.get_next, stats.set_next, stats.compare);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1 0");
}

TEST(Compiler, intXorThenUseAsIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[4];
            int i;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            a[3] = 40;
            i = 0;
            i ^= 2;
            printf("%d", a[i]);
            i ^= 1;
            printf(" %d", a[i]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("30 40");
}

TEST(Compiler, intAndThenUseAsIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[4];
            int i;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            a[3] = 40;
            i = 3;
            i &= 1;
            printf("%d", a[i]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, preferListXorOneThenCompare) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int prefer_list;
            prefer_list = 1;
            prefer_list ^= 1;
            printf("%d %d ", prefer_list, 0 < prefer_list);
            prefer_list ^= 1;
            printf("%d %d", prefer_list, 0 < prefer_list);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 1 1");
}

TEST(Compiler, pointerAssignedZeroAfterDirtySlotIsNull) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        int main() {
            int *p;
            int x;
            x = 1;
            dirty8(&p);
            p = 0;
            if (p) {
                printf("1");
            } else {
                printf("0");
            }
            p = &x;
            if (p) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("01");
}

TEST(Compiler, sizeTBitLoopFillsRanksByIntIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int ranks[8];
            unsigned long n;
            int k;
            for (k = 0; k < 8; k++) {
                ranks[k] = 0;
            }
            n = 0;
            for (k = 0; k < 4; k++) {
                unsigned long m;
                int i;
                for (i = 0, m = n;; i++, m >>= 1) {
                    if (m & 1) {
                        ranks[i] = ranks[i] + 1;
                    } else if (!m) {
                        break;
                    }
                }
                n++;
            }
            printf("%d %d %d %d", ranks[0], ranks[1], ranks[2], ranks[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 2 0 0");
}

TEST(Compiler, seventhAndEighthIntStackArgThenPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void show(int a, int b, int c, int d, int e, int f, int g, int h, char *s) {
            printf("%d %d %s", g, h, s);
        }
        int main() {
            show(1, 2, 3, 4, 5, 6, 7, 8, "ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8 ok");
}

TEST(Compiler, linkedListMergesortFourAndEightNodes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *malloc(unsigned long);
        struct node {
            int value;
            int rank;
            struct node *next;
        };
        static int cmp_nodes(const struct node *a, const struct node *b) {
            int av = a->value, bv = b->value;
            return (av > bv) - (av < bv);
        }
        static struct node *merge(struct node *list, struct node *other,
                int (*compare_fn)(const struct node *, const struct node *)) {
            struct node *result = list, *tail;
            int prefer_list = compare_fn(list, other) <= 0;
            if (!prefer_list) {
                result = other;
                { struct node *tmp = list; list = other; other = tmp; }
            }
            for (;;) {
                do {
                    tail = list;
                    list = list->next;
                    if (!list) {
                        tail->next = other;
                        return result;
                    }
                } while (compare_fn(list, other) < prefer_list);
                tail->next = other;
                prefer_list ^= 1;
                { struct node *tmp = list; list = other; other = tmp; }
            }
        }
        static void sort_list(struct node **listp,
                int (*compare_fn)(const struct node *, const struct node *)) {
            struct node *list = *listp;
            struct node *ranks[64];
            unsigned long n = 0;
            if (!list)
                return;
            for (;;) {
                int i;
                unsigned long m;
                struct node *next = list->next;
                if (next)
                    list->next = 0;
                for (i = 0, m = n;; i++, m >>= 1) {
                    if (m & 1) {
                        list = merge(ranks[i], list, compare_fn);
                    } else if (next) {
                        break;
                    } else if (!m) {
                        *listp = list;
                        return;
                    }
                }
                n++;
                ranks[i] = list;
                list = next;
            }
        }
        static struct node *build(int *vals, int n) {
            struct node *list;
            struct node **tail;
            struct node *curr;
            int i;
            tail = &list;
            for (i = 0; i < n; i++) {
                curr = malloc(sizeof(struct node));
                curr->value = vals[i];
                curr->rank = i;
                *tail = curr;
                tail = &curr->next;
            }
            *tail = 0;
            return list;
        }
        static void print_list(struct node *curr) {
            for (; curr; curr = curr->next)
                printf("%d ", curr->value);
        }
        int main() {
            int four[4];
            int eight[8];
            struct node *a;
            struct node *b;
            four[0] = 3; four[1] = 1; four[2] = 4; four[3] = 2;
            eight[0] = 3; eight[1] = 1; eight[2] = 4; eight[3] = 1;
            eight[4] = 5; eight[5] = 9; eight[6] = 2; eight[7] = 6;
            a = build(four, 4);
            sort_list(&a, cmp_nodes);
            print_list(a);
            b = build(eight, 8);
            sort_list(&b, cmp_nodes);
            print_list(b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4 1 1 2 3 4 5 6 9 ");
}

TEST(Compiler, manyLiveIntsAroundFourNodeMallocList) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *malloc(unsigned long);
        struct node {
            int value;
            struct node *next;
        };
        int main() {
            int a0, a1, a2, a3, a4, a5, a6, a7;
            int i;
            struct node *list;
            struct node **tail;
            struct node *curr;
            a0 = 1; a1 = 1; a2 = 1; a3 = 1;
            a4 = 1; a5 = 1; a6 = 1; a7 = 1;
            tail = &list;
            for (i = 0; i < 4; i++) {
                curr = malloc(sizeof(struct node));
                curr->value = 3 - i;
                *tail = curr;
                tail = &curr->next;
            }
            *tail = 0;
            printf("%d ", a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7);
            for (curr = list; curr; curr = curr->next)
                printf("%d ", curr->value);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 3 2 1 0 ");
}

TEST(Compiler, namedLocalBoolAssignedZeroAfterDirtyThenIf) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        int main() {
            bool b;
            dirty8(&b);
            b = 0;
            if (b) {
                printf("1");
            } else {
                printf("0");
            }
            b = 1;
            if (b) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("01");
}

TEST(Compiler, namedLocalIntAssignedZeroAfterDirtyThenIfAndLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        int main() {
            int n;
            dirty8(&n);
            n = 0;
            if (n) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d %ld", n == 0, (long)n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 0");
}

TEST(Compiler, dirtyIntLoadedAsLongIgnoresNeighborBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void seti(int *p, int v) {
            *p = v;
        }
        int main() {
            int n;
            dirty8(&n);
            seti(&n, 1);
            printf("%ld %d", (long)n, n == 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, dirtyIntsAddedThenUsedAsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void seti(int *p, int v) {
            *p = v;
        }
        int main() {
            int a;
            int b;
            dirty8(&a);
            dirty8(&b);
            seti(&a, 1);
            seti(&b, 2);
            printf("%ld", (long)(a + b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, dirtyIntAddThenUseAsIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void seti(int *p, int v) {
            *p = v;
        }
        int main() {
            int arr[4];
            int i;
            int one;
            arr[0] = 10;
            arr[1] = 20;
            arr[2] = 30;
            arr[3] = 40;
            dirty8(&i);
            dirty8(&one);
            seti(&i, 1);
            seti(&one, 2);
            printf("%d", arr[i + one]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("40");
}

TEST(Compiler, unsignedIntHighBitZeroExtendsToLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned int u;
            u = 0x80000000u;
            printf("%ld", (long)u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2147483648");
}

TEST(Compiler, signedCharMinusOneExtendsAfterDirtyStore) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void setc(char *p, int v) {
            *p = v;
        }
        int main() {
            char c;
            dirty8(&c);
            setc(&c, -1);
            printf("%d %ld", (int)c, (long)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, signedShortMinusOneExtendsAfterDirtyStore) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void sets(short *p, int v) {
            *p = v;
        }
        int main() {
            short s;
            dirty8(&s);
            sets(&s, -1);
            printf("%d %ld", (int)s, (long)s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, longTruncatedToIntThenExtendedBack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long x;
            int i;
            x = 0x100000001L;
            i = (int)x;
            printf("%d %ld", i, (long)i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, unsignedCharPostfixIncrementWrapsExpressionAndObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            unsigned char d;
            c = 255;
            d = c++;
            printf("%d %d ", (int)d, (int)c);
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("255 0 0");
}

TEST(Compiler, unsignedCharPrefixIncrementWrapsExpressionAndObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            unsigned char d;
            c = 255;
            d = ++c;
            printf("%d %d ", (int)d, (int)c);
            if (d) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 0");
}

TEST(Compiler, unsignedCharCompoundAddWrapsThenIfIsFalse) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            c = 255;
            c += 1;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, unsignedCharIncrementThenUseAsIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int arr[2];
            unsigned char c;
            arr[0] = 7;
            arr[1] = 9;
            c = 255;
            c++;
            printf("%d", arr[c]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, unsignedIntIncrementWrapsThenIfIsFalse) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned int u;
            u = 0xffffffffu;
            u++;
            if (u) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %u", u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, pointerStoredThroughPointerAfterDirtyIsNull) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        int main() {
            int *p;
            int **pp;
            int x;
            x = 1;
            dirty8(&p);
            pp = &p;
            *pp = 0;
            if (p) {
                printf("1");
            } else {
                printf("0");
            }
            *pp = &x;
            if (p) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("01");
}

TEST(Compiler, dirtyCharsComparedEqualAfterStoreZero) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void setc(char *p, int v) {
            *p = v;
        }
        int main() {
            char a;
            char b;
            dirty8(&a);
            dirty8(&b);
            setc(&a, 0);
            setc(&b, 0);
            printf("%d %d", a == b, a == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, unsignedCharPrefixIncrementReturnedThenIfIsFalse) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned char bump(unsigned char c) {
            return ++c;
        }
        int main() {
            unsigned char z;
            z = bump(255);
            if (z) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, dirtyIntNegatedThenUsedAsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void seti(int *p, int v) {
            *p = v;
        }
        int main() {
            int n;
            dirty8(&n);
            seti(&n, 2);
            printf("%ld", (long)(-n));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-2");
}

TEST(Compiler, pointerIncrementThenDerefKeepsNeighbors) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int arr[3];
            int *p;
            arr[0] = 10;
            arr[1] = 20;
            arr[2] = 30;
            p = arr;
            p++;
            printf("%d %d", *p, p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

TEST(Compiler, dirtyIntShiftedThenUsedAsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void seti(int *p, int v) {
            *p = v;
        }
        int main() {
            int n;
            dirty8(&n);
            seti(&n, 1);
            printf("%ld", (long)(n << 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, dereferenceSignedCharMinusOneAsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        int main() {
            char c;
            char *p;
            dirty8(&c);
            p = &c;
            *p = -1;
            printf("%d %ld", (int)*p, (long)*p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, manyLiveThenBoolIfAfterPointerStore) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void set(bool *p, int v) {
            *p = v;
        }
        int main() {
            int a0, a1, a2, a3, a4, a5, a6, a7;
            bool b;
            a0 = 1; a1 = 1; a2 = 1; a3 = 1;
            a4 = 1; a5 = 1; a6 = 1; a7 = 1;
            dirty8(&b);
            set(&b, 0);
            if (b) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 8");
}

TEST(Compiler, incrementThenAssignConstantTruncatesRegisterResidentUchar) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            c = 255;
            c++;
            c = 256;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, incrementThenAssignWiderIntTruncatesRegisterResidentUchar) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            int x;
            c = 255;
            x = 256;
            c++;
            c = x;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, incrementThenAssignFittingConstantKeepsRegisterResidentUchar) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            c = 255;
            c++;
            c = 1;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, incrementThenAssignConstantTruncatesRegisterResidentUshort) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned short s;
            s = 65535;
            s++;
            s = 65536;
            if (s) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, incrementThenAssignDoubleConstantTruncatesRegisterResidentUchar) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            c = 255;
            c++;
            c = 256.0;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, incrementThenAssignDoubleVariableTruncatesRegisterResidentUchar) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            double d;
            c = 255;
            d = 256.0;
            c++;
            c = d;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, incrementThenAssignFittingDoubleKeepsRegisterResidentUchar) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            c = 255;
            c++;
            c = 1.0;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, incrementThenAssignDoubleConstantTruncatesRegisterResidentUshort) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned short s;
            s = 65535;
            s++;
            s = 65536.0;
            if (s) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

} // namespace
