#include "TestFixtures.h"

namespace {

TEST(Compiler, compilesForLoopSumProgram) {
    SourceProgram program{R"prg(
        int forSum(int it, int add) {
            int i;
            int result = 0;
            for (i = 0; i < it; i++) {
                result = result + add;
            }
            return result;
        }

        int main() {
            printf("%d", forSum(5, 2));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10");
}

TEST(Compiler, forLoopIterationOutput) {
    SourceProgram program{R"prg(
        int iterationOutput(int n) {
            int i;
            for (i = 0; i <= n; i++) {
                printf("%d ", i);
            }
            return 0;
        }

        int main() {
            int n;
            scanf("%ld", &n);
            iterationOutput(n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10", "0 1 2 3 4 5 6 7 8 9 10 ");
    program.runAndExpect("5", "0 1 2 3 4 5 ");
    program.runAndExpect("1", "0 1 ");
    program.runAndExpect("0", "0 ");
    program.runAndExpect("-1", "");
}

TEST(Compiler, forLoopLessThan) {
    SourceProgram program{R"prg(
        int iterationOutput(int n) {
            int i;
            for (i = 0; i < n; i++) {
                printf("%d ", i);
            }
            return 0;
        }

        int main() {
            int n;
            scanf("%ld", &n);
            iterationOutput(n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10", "0 1 2 3 4 5 6 7 8 9 ");
    program.runAndExpect("5", "0 1 2 3 4 ");
    program.runAndExpect("1", "0 ");
    program.runAndExpect("0", "");
    program.runAndExpect("-1", "");
}

// C99 for-with-declaration: loop variable is declared in the header.
TEST(Compiler, forWithDeclarationInit) {
    SourceProgram program{R"prg(
        int main() {
            int sum;
            sum = 0;
            for (int i = 0; i < 5; i++) {
                sum = sum + i;
            }
            printf("%d", sum);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("10");
}

// Declaration in for-init is scoped to the loop (not visible after).
TEST(Compiler, forDeclarationScopedToLoop) {
    SourceProgram program{R"prg(
        int main() {
            for (int i = 0; i < 1; i++) {
                printf("%d", i);
            }
            int i;
            i = 7;
            printf(" %d", i);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("0 7");
}

// for (decl ;;) with empty clause and increment.
TEST(Compiler, forWithDeclEmptyClauseAndIncrement) {
    SourceProgram program{R"prg(
        int main() {
            for (int i = 0; ; ) {
                printf("%d", i);
                break;
            }
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("0");
}

} // namespace
