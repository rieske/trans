#include "TestFixtures.h"

namespace {

TEST(Compiler, compilesFibonacciProgram) {
    SourceProgram program{R"prg(
        int fib (int n1, int n2, int max) {
            int fibb = n1 + n2;
            printf("%d ", n2);
            if (fibb > max)
                return fibb;
            else
                return fib(n2, fibb, max);
        }

        int main() {
            int a;
            int b;
            int max;
            input max;
            a = 1;
            b = 1;
            printf("%d", fib(a, b, max));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("42", "1 2 3 5 8 13 21 34 55");
}

}
