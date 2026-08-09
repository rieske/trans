#include "TestFixtures.h"

namespace {

TEST(Compiler, unaryPlusPreserves) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d", +a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0", "0");
    program.runAndExpect("5", "5");
    program.runAndExpect("-2", "-2");
}

TEST(Compiler, chainedAssignment) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a;
            int b;
            int c;
            c = 3;
            a = b = c;
            printf("%d %d %d", a, b, c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 3 3");
}

TEST(Compiler, commaOperatorDiscardsLeft) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a;
            int b;
            b = 1;
            a = (b = 2, 3);
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 2");
}

TEST(Compiler, logicalAndSkipsRight) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            0 && ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, logicalOrSkipsRight) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            1 || ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, logicalAndEvaluatesRight) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            1 && ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, logicalOrEvaluatesRight) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            0 || ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, doubleNotOnComparison) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a;
            int b;
            scanf("%ld %ld", &a, &b);
            printf("%d", !!(a == b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 0", "0");
}

TEST(Compiler, pointerEquality) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a;
            int b;
            a = 1;
            b = 2;
            printf("%d %d", &a == &a, &a == &b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, expressionStatementArithmeticAndComparison) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a;
            int b;
            a = 3;
            b = 4;
            a * b + 1;
            a < b;
            a == b;
            (a + b);
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

TEST(Compiler, expressionStatementEvaluatesCallOperands) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int g;
        int bump(void) {
            g = g + 1;
            return 0;
        }
        int main() {
            g = 0;
            bump() + bump();
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

} // namespace
