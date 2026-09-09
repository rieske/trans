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

TEST(Compiler, commaOperatorResultDecaysToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int a[3];
        int main() {
            printf("%d %d %d", (int)sizeof((0, a)), (int)sizeof(a), (int)sizeof(typeof((0, a))));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 12 8");
}

TEST(Compiler, commaOperatorResultIndexesTheArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int a[3] = { 4, 5, 6 };
        int main() {
            int *p = (0, a);
            printf("%d %d", (0, a)[1], p[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6");
}

TEST(Compiler, commaOperatorResultIsNotAnLvalue) {
    SourceProgram program{R"prg(
        int main() {
            int x = 1;
            int y = 2;
            (y, x) = 5;
            return x;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required");
}

TEST(Compiler, commaOperatorOverAnArrayIsNotAssignable) {
    SourceProgram program{R"prg(
        int a[3];
        int b[3];
        int main() {
            int y = 1;
            int *q = b;
            (y, a) = q;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required");
}

TEST(Compiler, statementExpressionResultDecaysToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int a[3] = { 4, 5, 6 };
        int main() {
            printf("%d %d %d", (int)sizeof(({ a; })), (int)sizeof(a), ({ a; })[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 12 5");
}

TEST(Compiler, statementExpressionResultIsNotAnLvalue) {
    SourceProgram program{R"prg(
        int main() {
            int x = 1;
            int y = 2;
            ({ y; x; }) = 5;
            return x;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required");
}

TEST(Compiler, castsOfScalarAndDecayedOperandsStayLegal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g(void) { return 3; }
        int a[3] = { 1, 2, 3 };
        int main(void) {
            struct S { int m; } s = { 9 };
            int i = 5;
            int *p = a;
            double d = 2.5;
            (void)s;
            printf("%d %d %d %d", (char)i, (int)d, (int)(a != 0), (int)(g != 0));
            printf(" %d", (int)(p != 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 2 1 1 1");
}

TEST(Compiler, voidValuedExpressionsStayLegalWhereCAllowsThem) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void v(void) { }
        int c = 1;
        int main(void) {
            v();
            (void)v();
            c ? v() : v();
            printf("%d", (v(), 7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

} // namespace
