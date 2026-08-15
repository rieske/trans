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
