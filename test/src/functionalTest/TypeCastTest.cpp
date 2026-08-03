#include "TestFixtures.h"

namespace {

TEST(Compiler, castIntToPointerAndBack) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            int* p;
            x = 42;
            p = (int*)x;
            printf("%d", (int)p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, castPointerAndIndex) {
    SourceProgram program{R"prg(
        int main() {
            int a[2];
            int* p;
            a[0] = 1;
            a[1] = 2;
            p = &a[0];
            printf("%d", ((int*)p)[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, castArrayToPointer) {
    SourceProgram program{R"prg(
        int main() {
            int a[2];
            int* p;
            a[0] = 7;
            a[1] = 8;
            p = (int*)a;
            printf("%d %d", p[0], p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8");
}

TEST(Compiler, castMultiDimRowToPointer) {
    SourceProgram program{R"prg(
        int main() {
            int a[2][3];
            int* p;
            a[1][2] = 42;
            p = (int*)a[1];
            printf("%d", p[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}


TEST(Compiler, castConstantArrayBound) {
    SourceProgram program{R"prg(
        int main() {
            int a[(int)3];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            printf("%d %d", a[2], sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 12");
}

// Int-to-float conversion in assignment (SSE path).
TEST(Compiler, intToDoubleAssignment) {
    SourceProgram program{R"prg(
        int main() {
            int n = 21;
            double d;
            d = n;
            d = d + d;
            printf("%d", (int)d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Cast double → int local (cvttsd2si path).
TEST(Compiler, castDoubleToIntLocal) {
    SourceProgram program{R"prg(
        int main() {
            double d = 42.9;
            int n = (int)d;
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Cast to array type is error.
TEST(Compiler, castToArrayTypeIsError) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = 1;
            (int[4])x;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("cast to array");
}


} // namespace
