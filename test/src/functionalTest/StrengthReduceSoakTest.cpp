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

} // namespace
