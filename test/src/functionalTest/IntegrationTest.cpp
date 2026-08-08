#include "TestFixtures.h"

namespace {

TEST(Compiler, mixedControlFlowProgram) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, expressionStatementOnly) {
    SourceProgram program{R"prg(int printf(const char *, ...);
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

TEST(Compiler, expressionStatementArithmeticAndComparison) {
    SourceProgram program{R"prg(int printf(const char *, ...);
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

TEST(Compiler, expressionStatementCommaEvaluatesLeftToRight) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int show(int x) {
            printf("%d", x);
            return x;
        }
        int main() {
            show(1), show(2), show(3);
            printf(".");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("123.");
}

TEST(Compiler, expressionStatementShortCircuitSkipsRight) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int show(int x) {
            printf("%d", x);
            return x;
        }
        int main() {
            0 && show(8);
            1 && show(1);
            1 || show(9);
            0 || show(2);
            printf(".");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12.");
}

TEST(Compiler, expressionStatementTernaryAndSizeof) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int show(int x) {
            printf("%d", x);
            return x;
        }
        int main() {
            int a;
            a = 1;
            a ? show(3) : show(4);
            0 ? show(5) : show(6);
            sizeof(a);
            sizeof show(7);
            printf(".");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("36.");
}

TEST(Compiler, expressionStatementDerefAndControlBodies) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int *p;
            int i;
            a = 1;
            p = &a;
            *p;
            if (a)
                a + 1;
            else
                a - 1;
            i = 0;
            while (i < 1) {
                i + 1;
                i = i + 1;
            }
            for (i = 0; i < 1; i = i + 1)
                a * 2;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

} // namespace
