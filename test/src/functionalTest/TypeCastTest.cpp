#include "TestFixtures.h"

namespace {

TEST(Compiler, castIntToPointerAndBack) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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

} // namespace
