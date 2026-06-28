#include "TestFixtures.h"

namespace {

TEST(Compiler, mixedControlFlowProgram) {
    SourceProgram program{R"prg(
        int bump(int* p) {
            ++(*p);
            return *p;
        }

        int main() {
            int n;
            int i;
            int acc;
            n = 0;
            acc = 0;
            for (i = 0; i < 3; i = i + 1) {
                if (i < 2) {
                    acc = acc + bump(&n);
                } else {
                    acc = acc + n;
                }
            }
            while (n < 5) {
                n = n + 1;
            }
            printf("%d %d", acc, n);
            return 0;
        }
    )prg"};
    // i=0,1: bump twice -> n=2, acc=1+2=3; i=2: acc=3+2=5; while n to 5
    program.compile();
    program.runAndExpect("5 5");
}

TEST(Compiler, manyLocalsLiveAcrossCallsAcrossPrintf) {
    SourceProgram program{R"prg(
        int id(int x) {
            return x;
        }

        int main() {
            int a;
            int b;
            int c;
            int d;
            int e;
            int f;
            a = 1;
            b = 2;
            c = 3;
            d = 4;
            e = 5;
            f = 6;
            printf("%d ", id(a));
            printf("%d ", id(b));
            printf("%d ", id(c));
            printf("%d ", id(d));
            printf("%d ", id(e));
            printf("%d", id(f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4 5 6");
}

TEST(Compiler, deeplyNestedUnary) {
    // Space between minuses: two unary minus operators (not decrement).
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d %d", !!!!a, - -a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0", "0 0");
    program.runAndExpect("1", "1 1");
    program.runAndExpect("2", "1 2");
}

// TODO(gap): expression statement that is not an assignment/call/inc - e.g. `a + b;`.
// Parser/grammar only accepts a subset of statements after `;` expectations (error:
// unexpected `+`). Need statement production to accept general `<exp> ;` (valid C).
/*
TEST(Compiler, expressionStatementOnly) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            a = 1;
            b = 2;
            a + b;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}
*/

} // namespace
