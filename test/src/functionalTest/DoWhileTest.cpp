#include "TestFixtures.h"

namespace {

TEST(Compiler, doWhileRunsAtLeastOnce) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 0;
            do {
                printf("%d", 7);
                n = n + 1;
            } while (0);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("71");
}

TEST(Compiler, doWhileMultipleIterations) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 0;
            do {
                n = n + 1;
            } while (n < 3);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, doWhileBreak) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 0;
            do {
                n = n + 1;
                if (n == 2) {
                    break;
                }
            } while (1);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, doWhileContinue) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int s;
            n = 0;
            s = 0;
            do {
                n = n + 1;
                if (n == 2) {
                    continue;
                }
                s = s + n;
            } while (n < 3);
            printf("%d %d", n, s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

} // namespace
