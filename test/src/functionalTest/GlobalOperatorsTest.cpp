#include "TestFixtures.h"

namespace {


// A global operand must work in every arithmetic/bitwise/shift op, not just + and -,
// including the two-stored-operands path (two globals).
TEST(Compiler, twoGlobalsInMultiplication) {
    SourceProgram program{R"prg(
        int a;
        int b;
        int main() {
            a = 6;
            b = 7;
            printf("%d", a * b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}


TEST(Compiler, twoGlobalsInDivisionAndModulo) {
    SourceProgram program{R"prg(
        int a;
        int b;
        int main() {
            a = 20;
            b = 6;
            printf("%d %d", a / b, a % b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 2");
}


TEST(Compiler, twoGlobalsInBitwiseOps) {
    SourceProgram program{R"prg(
        int a;
        int b;
        int main() {
            a = 6;
            b = 3;
            printf("%d %d %d", a & b, a | b, a ^ b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 7 5");
}


TEST(Compiler, globalOperandInShifts) {
    SourceProgram program{R"prg(
        int a;
        int b;
        int main() {
            a = 8;
            b = 2;
            printf("%d %d", a << b, a >> b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 2");
}


// Shift count is loaded into scratch (%cl) without register-caching the global home;
// a later write must update memory so the caller sees it.
TEST(Compiler, globalWrittenAfterBeingUsedAsShiftCountIsVisibleInCaller) {
    SourceProgram program{R"prg(
        int g;
        int t;

        void use() {
            t = 1 << g;
            g = 5;
            return;
        }

        int main() {
            g = 2;
            use();
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}


// Signed division/modulo must sign-extend the dividend; a negative global dividend must work.
TEST(Compiler, signedDivisionAndModuloOfNegativeGlobal) {
    SourceProgram program{R"prg(
        int a;
        int b;
        int main() {
            a = -20;
            b = 6;
            printf("%d %d", a / b, a % b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-3 -2");
}


// Compound assignment makes the global the RESULT of the op (g op= b lowers to Op(g, b, g)).
// The computed value must reach [rel g], not stay register-resident.
TEST(Compiler, globalPlusEquals) {
    SourceProgram program{R"prg(
        int g;
        int main() {
            g = 10;
            g += 5;
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("15");
}


TEST(Compiler, globalMinusEquals) {
    SourceProgram program{R"prg(
        int g;
        int main() {
            g = 10;
            g -= 3;
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}


TEST(Compiler, globalTimesEquals) {
    SourceProgram program{R"prg(
        int g;
        int main() {
            g = 6;
            g *= 7;
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}


TEST(Compiler, globalDivideAndModuloEquals) {
    SourceProgram program{R"prg(
        int g;
        int h;
        int main() {
            g = 20;
            g /= 6;
            h = 20;
            h %= 6;
            printf("%d %d", g, h);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 2");
}


TEST(Compiler, globalShiftEquals) {
    SourceProgram program{R"prg(
        int g;
        int h;
        int main() {
            g = 1;
            g <<= 4;
            h = 64;
            h >>= 2;
            printf("%d %d", g, h);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16 16");
}


TEST(Compiler, globalBitwiseEquals) {
    SourceProgram program{R"prg(
        int g;
        int h;
        int k;
        int main() {
            g = 6;
            g &= 3;
            h = 6;
            h |= 1;
            k = 6;
            k ^= 3;
            printf("%d %d %d", g, h, k);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 7 5");
}


// A compound-assign write to a global must be visible across function boundaries.
TEST(Compiler, globalCompoundAssignVisibleAcrossFunction) {
    SourceProgram program{R"prg(
        int counter;

        void bump() {
            counter += 10;
            return;
        }

        int main() {
            counter = 0;
            bump();
            bump();
            printf("%d", counter);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}


// Globals must not be register-cached on their home Value. After a read, a write, and a call
// (spill of caller-saved regs), memory must still hold the written value - not a stale load.
TEST(Compiler, globalWriteAfterReadSurvivesCallSpill) {
    SourceProgram program{R"prg(
        int g;
        int t;

        void touch() {
            printf("%d", t);
            return;
        }

        int main() {
            g = 1;
            t = g;
            g = 2;
            touch();
            printf(" %d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}


// Two globals loaded for an expression; later assign one; call; reread.
TEST(Compiler, globalAssignAfterTwoGlobalExpressionSurvivesCall) {
    SourceProgram program{R"prg(
        int a;
        int b;

        void bump() {
            return;
        }

        int main() {
            a = 3;
            b = 4;
            printf("%d", a + b);
            a = 9;
            bump();
            printf(" %d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 9");
}


// Use global, compound-assign, print again (home must be updated; no stale cache).
TEST(Compiler, globalUseThenCompoundAssignThenRead) {
    SourceProgram program{R"prg(
        int g;
        int main() {
            g = 10;
            printf("%d", g);
            g += 5;
            printf(" %d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 15");
}


// Pass global by value into a callee (arg regs are scratch; home must not be bound).
TEST(Compiler, globalPassedByValueThenUpdated) {
    SourceProgram program{R"prg(
        int g;

        void f(int x) {
            printf("%d ", x);
            return;
        }

        int main() {
            g = 7;
            f(g);
            g = 8;
            f(g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8 ");
}

} // namespace
