#include "TestFixtures.h"

namespace {

TEST(Compiler, strengthReduceMulByInvariant) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += i * k;
            }
            return s;
        }

        int main() {
            printf("%d", soak(4, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("18");
}

TEST(Compiler, strengthReduceMulByStepTwo) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i += 2) {
                s += i * k;
            }
            return s;
        }

        int main() {
            printf("%d", soak(6, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("18");
}

TEST(Compiler, strengthReduceMulZeroTrip) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k) {
            int i;
            int s;
            s = 7;
            for (i = 1; i < n; i++) {
                s += i * k;
            }
            return s;
        }

        int main() {
            printf("%d", soak(0, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, strengthReduceMulFactorAliasedByStore) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k) {
            int i;
            int s;
            int *p;
            s = 0;
            p = &k;
            for (i = 0; i < n; i++) {
                *p = *p + 1;
                s = s + i * k;
            }
            return s;
        }

        int main() {
            printf("%d", soak(3, 1));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("11");
}

TEST(Compiler, strengthReduceIndexInts) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *a, int n) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += a[i];
            }
            return s;
        }

        int main() {
            int a[4];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            printf("%d", soak(a, 4));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("10");
}

TEST(Compiler, strengthReduceIndexStruct) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S { int x; char c; };
        int soak(struct S *a, int n) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += a[i].x;
            }
            return s;
        }

        int main() {
            struct S a[2];
            a[0].x = 10;
            a[0].c = 0;
            a[1].x = 7;
            a[1].c = 0;
            printf("%d", soak(a, 2));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("17");
}

TEST(Compiler, strengthReduceIndexZeroTrip) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *a, int n) {
            int i;
            int s;
            s = 7;
            for (i = 1; i < n; i++) {
                s += a[i];
            }
            return s;
        }

        int main() {
            int a[2];
            a[0] = 1;
            a[1] = 5;
            printf("%d", soak(a, 0));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, strengthReduceIndexBaseAdvances) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *a, int n) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += a[i];
                a = a + 1;
            }
            return s;
        }

        int main() {
            int a[5];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            a[4] = 5;
            printf("%d", soak(a, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, strengthReduceNarrowIndexWraps) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *a) {
            unsigned char i;
            int s;
            s = 0;
            i = 255;
            do {
                s += a[i];
                i++;
            } while (i != 1);
            return s;
        }

        int main() {
            int a[257];
            int i;
            for (i = 0; i < 257; i++) {
                a[i] = 0;
            }
            a[0] = 3;
            a[255] = 5;
            a[256] = 100;
            printf("%d", soak(a));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, strengthReduceIndexAcrossCall) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        static int slide(int **p) {
            *p = *p + 1;
            return 0;
        }

        int soak(int *a, int n) {
            long i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += a[i];
                slide(&a);
            }
            return s;
        }

        int main() {
            int a[5];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            a[4] = 5;
            printf("%d", soak(a, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, strengthReduceAddressTakenIv) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n) {
            int i;
            int s;
            int *p;
            s = 0;
            p = &i;
            for (i = 0; i < n; i++) {
                s += i * 2;
                *p += 10;
            }
            return s;
        }

        int main() {
            printf("%d", soak(30));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("66");
}

TEST(Compiler, strengthReduceFactorOnTheLeft) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += k * i;
            }
            return s;
        }

        int main() {
            printf("%d", soak(4, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("18");
}

TEST(Compiler, strengthReduceDecrementMul) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k) {
            int i;
            int s;
            s = 0;
            for (i = n; i > 0; i--) {
                s += i * k;
            }
            return s;
        }

        int main() {
            printf("%d", soak(4, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("30");
}

TEST(Compiler, strengthReduceSubtractStep) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k) {
            int i;
            int s;
            s = 0;
            for (i = n; i > 0; i -= k) {
                s += i * 3;
            }
            return s;
        }

        int main() {
            printf("%d", soak(6, 2));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("36");
}

TEST(Compiler, strengthReduceDecrementIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *a, int n) {
            long i;
            int s;
            s = 0;
            for (i = n - 1; i >= 0; i--) {
                s += a[i];
            }
            return s;
        }

        int main() {
            int a[4];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            printf("%d", soak(a, 4));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("10");
}

TEST(Compiler, strengthReducePointerPlusIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *p, int n) {
            long i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += *(p + i);
            }
            return s;
        }

        int main() {
            int a[4];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            printf("%d", soak(a, 4));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("10");
}

TEST(Compiler, strengthReducePointerMinusIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *p, long n) {
            long i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += *(p - i);
            }
            return s;
        }

        int main() {
            int a[4];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            printf("%d", soak(a + 3, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, strengthReduceIndexUsedTwice) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *a, int n) {
            long i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += a[i] + a[i];
            }
            return s;
        }

        int main() {
            int a[4];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            printf("%d", soak(a, 4));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, strengthReduceIndexStepTwoStaysScaled) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *a, int n) {
            long i;
            int s;
            s = 0;
            for (i = 0; i < n; i += 2) {
                s += a[i];
            }
            return s;
        }

        int main() {
            int a[6];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            a[4] = 5;
            a[5] = 6;
            printf("%d", soak(a, 6));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, strengthReduceLongIndexBaseAdvances) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int *a, int n) {
            long i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += a[i];
                a = a + 1;
            }
            return s;
        }

        int main() {
            int a[5];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            a[4] = 5;
            printf("%d", soak(a, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, strengthReduceSameFactorTwice) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += i * k;
                s += i * k;
            }
            return s;
        }

        int main() {
            printf("%d", soak(4, 3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("36");
}

TEST(Compiler, strengthReduceTwoFactors) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k, int m) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += i * k + i * m;
            }
            return s;
        }

        int main() {
            printf("%d", soak(4, 3, 5));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("48");
}

TEST(Compiler, strengthReduceTwoInductionVariables) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n, int k, int m) {
            int i;
            int j;
            int s;
            s = 0;
            j = 0;
            for (i = 0; i < n; i++) {
                s += i * k;
                j++;
                s += j * m;
            }
            return s;
        }

        int main() {
            printf("%d", soak(3, 2, 4));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("30");
}

TEST(Compiler, originalIndexNegativeBoundDoesNotLoad) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(int *a, int n) {
            long i;
            int s;
            s = 7;
            for (i = 0; i < n; i++) {
                s += a[i];
            }
            return s;
        }
        int main(void) {
            printf("%d", f(0, -1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, originalIndexStartsAtOneStillSums) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(int *a, int n) {
            long i;
            int s;
            s = 0;
            for (i = 1; i < n; i++) {
                s += a[i];
            }
            return s;
        }
        int main(void) {
            int a[3];
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            printf("%d", f(a, 3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("50");
}

TEST(Compiler, overwrittenZeroDoesNotStartTheIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(int *a, long n) {
            int bytes;
            long t;
            long u;
            long i;
            int s;
            bytes = 1;
            char buf[bytes];
            buf[0] = 0;
            s = 0;
            t = 0;
            u = t;
            t = 1;
            i = t;
            for (; i < n; i++) {
                s += a[i];
                if (s) {
                    break;
                }
            }
            return s + buf[0] + (int)u;
        }
        int main(void) {
            int a[2];
            a[0] = 1;
            a[1] = 2;
            printf("%d", f(a, 1L << 62));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, unsignedCompareOfNegativeIntStillEnters) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(int *a, int n) {
            unsigned long i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s = a[i];
                if (s) {
                    break;
                }
            }
            return s;
        }
        int g(int *a, int n) {
            long i;
            int s;
            s = 0;
            for (i = 0; i < (unsigned long)n; i++) {
                s = a[i];
                if (s) {
                    break;
                }
            }
            return s;
        }
        int main(void) {
            int a[1];
            a[0] = 1;
            printf("%d%d", f(a, -1), g(a, -1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}

TEST(Compiler, hugeElementUnsignedBoundStillEnters) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { char b[2147483647]; };
        int f(struct S *a, unsigned n) {
            long i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                if (i == 1) {
                    s = (int)((char *)(a + i) - (char *)a) == 2147483647;
                    break;
                }
            }
            return s;
        }
        int main(void) {
            char dummy;
            printf("%d%d%d", f((struct S *)&dummy, 2u), f((struct S *)&dummy, 2147483647u),
                    f((struct S *)&dummy, 4294967295u));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("111");
}

TEST(Compiler, highSignedBaseStillCounts) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(char *a) {
            long i;
            int s;
            char *seen;
            s = 0;
            seen = 0;
            for (i = 0; i < 0x2000; i++) {
                seen = a + i;
                s = s + 1;
            }
            if (seen != a + 0x1fff) {
                return 0;
            }
            return s;
        }
        int main(void) {
            printf("%d", f((char *)0x7ffffffffffff000UL));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8192");
}

TEST(Compiler, highUnsignedBaseStillCounts) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(int *a) {
            unsigned long i;
            int s;
            s = 0;
            for (i = 0; i < 4; i++) {
                if (a + i != (int *)(0xfffffffffffffff0UL + 4 * i)) {
                    return 0;
                }
                s = s + 1;
            }
            return s;
        }
        int main(void) {
            printf("%d", f((int *)0xfffffffffffffff0UL));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

} // namespace
