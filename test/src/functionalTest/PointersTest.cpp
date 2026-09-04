#include "TestFixtures.h"

#include <fstream>

namespace {

TEST(Compiler, addAfterStoreThroughPointerUsesMemory) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a = 1;
            int *p = &a;
            *p = 2;
            printf("%d", a + 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, addAfterCalleeWritesThroughPointerUsesMemory) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int set(int *p) {
            *p = 2;
            return 0;
        }
        int main() {
            int a = 1;
            set(&a);
            printf("%d", a + 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, writesThroughPointerArgumentBeforeAnyCall) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int b;
            int* p;
            int** pp;
            a = 5;
            b = 7;
            p = &a;
            pp = &p;
            printf("%d %d", **pp, *p);
            **pp = 9;
            printf(" %d", a);
            *pp = &b;
            printf(" %d %d", *p, **pp);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 5 9 7 7");
}

TEST(Compiler, pointerToPointerInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int *p;
            int **pp = &p;
            a = 6;
            p = &a;
            printf("%d", **pp);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, pointerToPointerToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int *p;
            int **pp;
            int ***ppp;
            a = 3;
            p = &a;
            pp = &p;
            ppp = &pp;
            printf("%d", ***ppp);
            ***ppp = 8;
            printf(" %d %d", **pp, a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 8 8");
}

TEST(Compiler, pointerToPointerAsParameterAndReturn) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int **choose(int **x, int **y) {
            return x;
        }
        void retarget(int **pp, int *q) {
            *pp = q;
        }
        int main() {
            int a;
            int b;
            int *p;
            int **pp;
            int **out;
            a = 1;
            b = 2;
            p = &a;
            pp = &p;
            out = choose(pp, pp);
            printf("%d", **out);
            retarget(pp, &b);
            printf(" %d %d", **pp, *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 2");
}

TEST(Compiler, pointerToPointerArithmeticScalesByPointerSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int b;
            int *slot[2];
            int **p;
            a = 10;
            b = 20;
            slot[0] = &a;
            slot[1] = &b;
            p = &slot[0];
            printf("%d ", **p);
            printf("%d ", **(p + 1));
            p++;
            printf("%d", **p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 20 20");
}

TEST(Compiler, pointerToPointerAssignedFromNullConstant) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int **pp;
            pp = 0;
            printf("%d", pp == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, pointerToPointerAssignedFromRecordIsError) {
    SourceProgram program{R"prg(
        struct S { int x; };
        int main() {
            struct S s;
            int **pp;
            s.x = 1;
            pp = s;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// Pointer arithmetic must scale by pointee size (int stride 4). Found by targeted probing
// during mutfuzz campaign: p+1 / p-q used raw byte math.

TEST(Compiler, pointerPlusIntScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[0];
            printf("%d ", *(p + 1));
            printf("%d", *(p + 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

TEST(Compiler, pointerMinusPointerIsElementCount) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            int *q;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = &a[2];
            q = &a[0];
            printf("%d", p - q);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, pointerIncrementScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[0];
            p++;
            printf("%d ", *p);
            ++p;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

// int + ptr is commutative with ptr + int (IntPlusPtr IR form).
TEST(Compiler, intPlusPointerScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[0];
            printf("%d ", *(1 + p));
            printf("%d", *(2 + p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

// ptr - int scales the index (PtrMinusInt IR form).
TEST(Compiler, pointerMinusIntScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[2];
            printf("%d ", *(p - 1));
            printf("%d", *(p - 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 10");
}

// Pointer -- / prefix -- step by pointee size (non-unit Dec quad).
TEST(Compiler, pointerDecrementScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[2];
            p--;
            printf("%d ", *p);
            --p;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 10");
}

TEST(Compiler, charPointerDifferenceIsBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            char a[3];
            char *p;
            char *q;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = &a[2];
            q = &a[0];
            printf("%d", p - q);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, pointerDifferenceWorksWhenPointersLiveInMemory) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int *gp;
        int *gq;
        int main() {
            int a[3];
            int *p;
            int *q;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            gp = &a[2];
            gq = &a[0];
            p = &a[2];
            q = &a[0];
            printf("%d ", gp - gq);
            printf("%d ", gp - q);
            printf("%d", p - gq);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 2 2");
}

TEST(Compiler, pointerNegativeSubscriptIsPreviousElement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char s[4];
            char *p;
            s[0] = 'a';
            s[1] = 'b';
            s[2] = 'c';
            s[3] = 0;
            p = s + 2;
            printf("%c", p[-1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("b");
}

TEST(Compiler, intPointerNegativeSubscriptScales) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = a + 2;
            printf("%d", p[-1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, pointerPlusNegativeInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            int n;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = a + 2;
            n = -1;
            printf("%d", *(p + n));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, walkBackToGreaterThanLikeParseCommitDate) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char line[32];
            char *buf;
            char *eol;
            char *dateptr;
            line[0] = 't';
            line[1] = ' ';
            line[2] = '<';
            line[3] = 'e';
            line[4] = '>';
            line[5] = ' ';
            line[6] = '1';
            line[7] = '\n';
            line[8] = 0;
            buf = line;
            eol = line;
            while (*eol != '\n') {
                eol = eol + 1;
            }
            dateptr = eol;
            while (dateptr > buf && dateptr[-1] != '>') {
                dateptr = dateptr - 1;
            }
            printf("%c%c", dateptr[-1], dateptr[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("> ");
}

// p += n / p -= n must scale by pointee size. Fuzz: += used unscaled integer add
// (++p was already correct).
TEST(Compiler, pointerPlusEqualScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[4];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            a[3] = 40;
            p = &a[0];
            p += 1;
            printf("%d %d ", *p, (int)(p - a));
            p += 2;
            printf("%d %d", *p, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 1 40 3");
}

TEST(Compiler, pointerMinusEqualScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[4];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            a[3] = 40;
            p = &a[3];
            p -= 1;
            printf("%d %d ", *p, (int)(p - a));
            p -= 2;
            printf("%d %d", *p, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("30 2 10 0");
}

TEST(Compiler, shortPointerPlusEqualScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            short a[3];
            short *p;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = a;
            p += 1;
            printf("%d %d", (int)*p, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

TEST(Compiler, structPointerPlusEqualScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; int y; };
        int main() {
            struct S a[2];
            struct S *p;
            a[0].x = 1; a[0].y = 2;
            a[1].x = 3; a[1].y = 4;
            p = a;
            p += 1;
            printf("%d %d %d", p->x, p->y, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4 1");
}

TEST(Compiler, charPointerPlusEqualStepsByOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char a[3];
            char *p;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = a;
            p += 1;
            printf("%d %d", (int)*p, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

// C 6.5.2.1: E1[E2] is *((E1)+(E2)), so 1[p] == p[1].
TEST(Compiler, integerSubscriptOfPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            p = a;
            printf("%d %d %d", 0[p], 1[p], 2[p]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 5 6");
}

TEST(Compiler, integerSubscriptOfArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            printf("%d %d", 1[a], 2[a]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6");
}

TEST(Compiler, integerSubscriptStaticArrayAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int a[3];
        int *p = &1[a];
        int main() {
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

} // namespace
