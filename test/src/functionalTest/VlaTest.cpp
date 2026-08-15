#include "TestFixtures.h"

namespace {

TEST(Compiler, vlaLocalArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 3;
            int a[n];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            printf("%d %d", a[0], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3");
}

TEST(Compiler, vlaPointerToVla) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int (*p)[n];
            n = 3;
            int a[n];
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            p = &a;
            printf("%d %d", (*p)[0], (*p)[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 6");
}

TEST(Compiler, vlaPointerPlusInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int (*p)[n];
            int *q;
            n = 3;
            int a[2][n];
            q = &a[0][0];
            q[0] = 1;
            q[1] = 2;
            q[2] = 3;
            q[3] = 4;
            q[4] = 5;
            q[5] = 6;
            p = a;
            printf("%d %d", (*(p + 1))[0], (*(p + 1))[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 6");
}

TEST(Compiler, vlaIntPlusPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int (*p)[n];
            int *q;
            n = 3;
            int a[2][n];
            q = &a[0][0];
            q[0] = 1;
            q[1] = 2;
            q[2] = 3;
            q[3] = 4;
            q[4] = 5;
            q[5] = 6;
            p = a;
            printf("%d %d", (*(1 + p))[0], (*(1 + p))[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 6");
}

TEST(Compiler, vlaPointerIncrement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int (*p)[n];
            int *q;
            n = 3;
            int a[3][n];
            q = &a[0][0];
            q[0] = 1;
            q[1] = 2;
            q[2] = 3;
            q[3] = 4;
            q[4] = 5;
            q[5] = 6;
            q[6] = 7;
            q[7] = 8;
            q[8] = 9;
            p = a;
            p++;
            printf("%d ", (*p)[0]);
            ++p;
            printf("%d", (*p)[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 7");
}

TEST(Compiler, vlaPointerDecrement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int (*p)[n];
            n = 3;
            int a[9];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            a[4] = 5;
            a[5] = 6;
            a[6] = 7;
            a[7] = 8;
            a[8] = 9;
            p = (int (*)[n])&a[6];
            p--;
            printf("%d ", (*p)[0]);
            --p;
            printf("%d", (*p)[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 1");
}

TEST(Compiler, vlaPointerMinusPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int (*p)[n];
            int (*q)[n];
            n = 3;
            int a[6];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            a[4] = 5;
            a[5] = 6;
            p = (int (*)[n])&a[3];
            q = (int (*)[n])&a[0];
            printf("%d", (int)(p - q));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, vlaPointerMinusInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int (*p)[n];
            n = 3;
            int a[6];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            a[3] = 4;
            a[4] = 5;
            a[5] = 6;
            p = (int (*)[n])&a[3];
            printf("%d %d", (*(p - 1))[0], (*(p - 1))[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3");
}

TEST(Compiler, vlaPointerSubscript) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int (*p)[n];
            int *q;
            n = 3;
            int a[2][n];
            q = &a[0][0];
            q[0] = 1;
            q[1] = 2;
            q[2] = 3;
            q[3] = 4;
            q[4] = 5;
            q[5] = 6;
            p = a;
            printf("%d %d", p[1][0], p[1][2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 6");
}

TEST(Compiler, vla2dInnerVariable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            int *q;
            n = 3;
            int a[2][n];
            q = &a[0][0];
            q[0] = 1;
            q[1] = 2;
            q[2] = 3;
            q[3] = 4;
            q[4] = 5;
            q[5] = 6;
            printf("%d %d", a[1][0], a[1][2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 6");
}

TEST(Compiler, sizeofVlaObjectIsElementCount) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 4;
            int a[n];
            a[0] = 1;
            printf("%d", (int)sizeof(a) / (int)sizeof(a[0]));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, vlaNestedInnerConstant) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 2;
            int a[n][2];
            a[0][0] = 1;
            a[0][1] = 2;
            a[1][0] = 3;
            a[1][1] = 4;
            printf("%d %d %d", a[0][0], a[1][1], (int)sizeof(a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 4 16");
}

TEST(Compiler, fileScopeVlaIsSemanticError) {
    SourceProgram program{R"prg(
        int n;
        int a[n];
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is not a non-negative constant expression");
}

TEST(Compiler, staticVlaIsSemanticError) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 3;
            static int a[n];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is not a non-negative constant expression");
}

} // namespace
