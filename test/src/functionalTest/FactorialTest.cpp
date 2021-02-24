#include "TestFixtures.h"

namespace {

TEST(Compiler, compilesWhileLoopFactorialProgram) {
    SourceProgram program{R"prg(
        int factorialWhile(int n) {
            int result = 1;
            while(n > 1) {
                result = result * n;
                n = n - 1;
            }
            return result;
        }

        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d", factorialWhile(a));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("5", "120");
    program.runAndExpect("1", "1");
    program.runAndExpect("0", "1");
    program.runAndExpect("-1", "1");
}

TEST(Compiler, compilesForLoopFactorialProgram) {
    SourceProgram program{R"prg(
        int factorialFor(int n) {
            int i;
            int result = 1;
            for (i = 1; i <= n; i++) {
                result *= i;
            }
            return result;
        }

        int main() {
            int n;
            scanf("%ld", &n);
            printf("%d", factorialFor(n));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("5", "120");
    program.runAndExpect("1", "1");
    program.runAndExpect("0", "1");
    program.runAndExpect("-1", "1");
}

}
