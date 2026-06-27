#include "TestFixtures.h"

namespace {

TEST(Compiler, unaryMinusOnVariable) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d %d", -a, -(-a));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0 0");
    program.runAndExpect("1", "-1 1");
    program.runAndExpect("-2", "2 -2");
}

TEST(Compiler, addressOfAndDereference) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int* p;
            scanf("%ld", &a);
            p = &a;
            printf("%d %d", *p, a);
            *p = 7;
            printf(" %d %d", *p, a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("3", "3 3 7 7");
    program.runAndExpect("0", "0 0 7 7");
}

TEST(Compiler, logicalNotOnVariable) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d", !a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "1");
    program.runAndExpect("1", "0");
    program.runAndExpect("2", "0");
    program.runAndExpect("-1", "0");
}

} // namespace
