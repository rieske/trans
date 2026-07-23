#include "TestFixtures.h"

namespace {

TEST(Compiler, gotoSkipsCode) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            goto end;
            a = 2;
        end:
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, gotoLoop) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 0;
        loop:
            n = n + 1;
            if (n < 3) {
                goto loop;
            }
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, gotoForwardIntoBlock) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 0;
            goto mid;
            a = 1;
        mid:
            a = a + 10;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10");
}

// Label on C99 for-with-declaration (git: cleanup: for (size_t i = 0; ...)).
// Regression: leftover '=' from init_declarator poisoned the label terminal stack.
TEST(Compiler, gotoLabelOnForWithDeclaration) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = 0;
            goto cleanup;
            x = 1;
        cleanup:
            for (int i = 0; i < 1; i = i + 1)
                x = x + 10;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10");
}

TEST(Compiler, gotoLabelOnForWithDeclAndEmptyBodyIncrement) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            int n;
            x = 0;
            n = 3;
            if (x)
                goto cleanup;
            x = 1;
        cleanup:
            for (int i = 0; i < n; i = i + 1) {
                x = x + 1;
            }
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

} // namespace
