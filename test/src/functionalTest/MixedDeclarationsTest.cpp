#include "TestFixtures.h"

namespace {

// C99: declarations may appear after statements in a block.
TEST(Compiler, mixedDeclarationsAfterStatement) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            int b;
            b = 2;
            printf("%d", a + b);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("3");
}

// Declaration after expression statement, used by va_start rewrite patterns.
TEST(Compiler, declarationAfterExpressionStatement) {
    SourceProgram program{R"prg(
        int main() {
            int base;
            base = 10;
            void * ap;
            ((ap) = (void *)((char *)&(base) + 0));
            int x;
            x = 32;
            printf("%d", base + x);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("42");
}

} // namespace
