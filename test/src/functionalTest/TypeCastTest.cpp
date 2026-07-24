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

} // namespace
