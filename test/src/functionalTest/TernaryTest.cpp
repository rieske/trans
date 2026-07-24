#include "TestFixtures.h"

namespace {

// Proves conditionalExpression is required: master throws until implemented.
TEST(Compiler, ternaryBasicTrue) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1 ? 10 : 20;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10");
}

TEST(Compiler, ternaryBasicFalse) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 0 ? 10 : 20;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, ternaryWithVariables) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            int y;
            int z;
            x = 5;
            y = 3;
            z = (x > y) ? x : y;
            printf("%d", z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, ternaryNested) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1 ? (0 ? 1 : 2) : 3;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, ternaryInExpression) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 2 + (1 ? 3 : 4);
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, ternaryOnlyOneArmEvaluated) {
    // Side effects: only the selected arm must run (via assignment to out).
    SourceProgram program{R"prg(
        int main() {
            int out;
            int unused;
            out = 0;
            unused = 1 ? (out = 7) : (out = 9);
            printf("%d %d", out, unused);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 7");
}

// Global init folds constant ternary via evaluateConstant.
TEST(Compiler, ternaryConstantGlobalInit) {
    SourceProgram program{R"prg(
        int g = 1 ? 4 : 5;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

// Selected arm's false path must not execute either.
TEST(Compiler, ternaryFalseArmOnly) {
    SourceProgram program{R"prg(
        int main() {
            int out;
            int unused;
            out = 0;
            unused = 0 ? (out = 7) : (out = 9);
            printf("%d %d", out, unused);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 9");
}

} // namespace
