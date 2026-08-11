#include "TestFixtures.h"

namespace {

TEST(Compiler, equals) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a == b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 1", "1");
    program.runAndExpect("42 42", "1");
    program.runAndExpect("-1 -1", "1");
    program.runAndExpect("-42 -42", "1");

    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 0", "0");
    program.runAndExpect("0 -1", "0");
    program.runAndExpect("-1 0", "0");
    program.runAndExpect("42 -42", "0");
    program.runAndExpect("-42 42", "0");
}

TEST(Compiler, notEquals) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a != b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0");
    program.runAndExpect("1 1", "0");
    program.runAndExpect("42 42", "0");
    program.runAndExpect("-1 -1", "0");
    program.runAndExpect("-42 -42", "0");

    program.runAndExpect("0 1", "1");
    program.runAndExpect("1 0", "1");
    program.runAndExpect("0 -1", "1");
    program.runAndExpect("-1 0", "1");
    program.runAndExpect("42 -42", "1");
    program.runAndExpect("-42 42", "1");
}

TEST(Compiler, equalsNegated) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", !(a == b));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0");
    program.runAndExpect("1 1", "0");
    program.runAndExpect("42 42", "0");
    program.runAndExpect("-1 -1", "0");
    program.runAndExpect("-42 -42", "0");

    program.runAndExpect("0 1", "1");
    program.runAndExpect("1 0", "1");
    program.runAndExpect("0 -1", "1");
    program.runAndExpect("-1 0", "1");
    program.runAndExpect("42 -42", "1");
    program.runAndExpect("-42 42", "1");
}

TEST(Compiler, notEqualsNegated) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", !(a != b));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 1", "1");
    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 0", "0");
}

TEST(Compiler, lessThanNegated) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", !(a < b));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 0", "1");
    program.runAndExpect("1 1", "1");
    program.runAndExpect("-1 0", "0");
}

TEST(Compiler, logicalAndNegated) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", !(a && b));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 0", "1");
    program.runAndExpect("0 1", "1");
    program.runAndExpect("1 1", "0");
}

TEST(Compiler, logicalOrNegated) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", !(a || b));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 0", "0");
    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 1", "0");
}

TEST(Compiler, doubleLogicalNot) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d", !!a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0");
    program.runAndExpect("1", "1");
    program.runAndExpect("2", "1");
    program.runAndExpect("-1", "1");
}

TEST(Compiler, lessThanOrEqualsConst) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d ", 0 <= 0);
            printf("%d ", 1 <= 1);
            printf("%d ", 42 <= 42);
            printf("%d ", -1 <= -1);
            printf("%d ", -42 <= -42);

            printf("%d ", 0 <= 1);
            printf("%d ", 1 <= 0);
            printf("%d ", 0 <= -1);
            printf("%d ", -1 <= 0);
            printf("%d ", 42 <= -42);
            printf("%d", -42 <= 42);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 1 1 1 1 1 0 0 1 0 1");
}

TEST(Compiler, lessThanOrEquals) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a <= b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 1", "1");
    program.runAndExpect("42 42", "1");
    program.runAndExpect("-1 -1", "1");
    program.runAndExpect("-42 -42", "1");

    program.runAndExpect("0 1", "1");
    program.runAndExpect("1 0", "0");
    program.runAndExpect("0 -1", "0");
    program.runAndExpect("-1 0", "1");
    program.runAndExpect("42 -42", "0");
    program.runAndExpect("-42 42", "1");
}

TEST(Compiler, lessThanConst) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d ", 0 < 0);
            printf("%d ", 1 < 1);
            printf("%d ", 42 < 42);
            printf("%d ", -1 < -1);
            printf("%d ", -42 < -42);

            printf("%d ", 0 < 1);
            printf("%d ", 1 < 0);
            printf("%d ", 0 < -1);
            printf("%d ", -1 < 0);
            printf("%d ", 42 < -42);
            printf("%d", -42 < 42);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0 0 0 0 1 0 0 1 0 1");
}

TEST(Compiler, lessThan) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a < b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0");
    program.runAndExpect("1 1", "0");
    program.runAndExpect("42 42", "0");
    program.runAndExpect("-1 -1", "0");
    program.runAndExpect("-42 -42", "0");

    program.runAndExpect("0 1", "1");
    program.runAndExpect("1 0", "0");
    program.runAndExpect("0 -1", "0");
    program.runAndExpect("-1 0", "1");
    program.runAndExpect("42 -42", "0");
    program.runAndExpect("-42 42", "1");
}

TEST(Compiler, moreThanConst) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d ", 0 > 0);
            printf("%d ", 1 > 1);
            printf("%d ", 42 > 42);
            printf("%d ", -1 > -1);
            printf("%d ", -42 > -42);

            printf("%d ", 0 > 1);
            printf("%d ", 1 > 0);
            printf("%d ", 0 > -1);
            printf("%d ", -1 > 0);
            printf("%d ", 42 > -42);
            printf("%d", -42 > 42);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0 0 0 0 0 1 1 0 1 0");
}

TEST(Compiler, moreThan) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a > b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0");
    program.runAndExpect("1 1", "0");
    program.runAndExpect("42 42", "0");
    program.runAndExpect("-1 -1", "0");
    program.runAndExpect("-42 -42", "0");

    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 0", "1");
    program.runAndExpect("0 -1", "1");
    program.runAndExpect("-1 0", "0");
    program.runAndExpect("42 -42", "1");
    program.runAndExpect("-42 42", "0");
}

TEST(Compiler, moreThanOrEqualsConst) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d ", 0 >= 0);
            printf("%d ", 1 >= 1);
            printf("%d ", 42 >= 42);
            printf("%d ", -1 >= -1);
            printf("%d ", -42 >= -42);

            printf("%d ", 0 >= 1);
            printf("%d ", 1 >= 0);
            printf("%d ", 0 >= -1);
            printf("%d ", -1 >= 0);
            printf("%d ", 42 >= -42);
            printf("%d", -42 >= 42);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 1 1 1 1 0 1 1 0 1 0");
}

TEST(Compiler, moreThanOrEquals) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a >= b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 1", "1");
    program.runAndExpect("42 42", "1");
    program.runAndExpect("-1 -1", "1");
    program.runAndExpect("-42 -42", "1");

    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 0", "1");
    program.runAndExpect("0 -1", "1");
    program.runAndExpect("-1 0", "0");
    program.runAndExpect("42 -42", "1");
    program.runAndExpect("-42 42", "0");
}

TEST(Compiler, ifEquality) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            if (a == b) {
                printf("%d", 1);
            } else {
                printf("%d", 0);
            }
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 1", "1");
    program.runAndExpect("42 42", "1");
    program.runAndExpect("-1 -1", "1");
    program.runAndExpect("-42 -42", "1");

    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 0", "0");
    program.runAndExpect("0 -1", "0");
    program.runAndExpect("-1 0", "0");
    program.runAndExpect("42 -42", "0");
    program.runAndExpect("-42 42", "0");
    program.runAndExpect("10 11", "0");
}

TEST(Compiler, simpleIfConditional) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int i;
            scanf("%ld", &i);
            if (i == 0) {
                printf("%d", 0);
            }
            if (i < 0) {
                printf("%d", -1);
            }
            if (i > 0) {
                printf("%d", 1);
            }
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0");
    program.runAndExpect("1", "1");

    program.runAndExpect("-1", "-1");
}

// ident.c: if (a) stmt; else if (b) { ... }  -- then matched, else unmatched.
TEST(Compiler, elseIfWithoutFinalElseThenIsStatement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int choose(int a, int b) {
            if (a)
                return 1;
            else if (b)
                return 2;
            return 3;
        }
        int main() {
            printf("%d %d %d", choose(1, 0), choose(0, 1), choose(0, 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, elseIfWithoutFinalElseThenIsBlock) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int choose(int a, int b) {
            if (a) {
                return 1;
            } else if (b)
                return 2;
            return 3;
        }
        int main() {
            printf("%d %d %d", choose(1, 0), choose(0, 1), choose(0, 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, elseIfWithoutFinalElseElifIsBlock) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int choose(int a, int b) {
            if (a)
                return 1;
            else if (b) {
                return 2;
            }
            return 3;
        }
        int main() {
            printf("%d %d %d", choose(1, 0), choose(0, 1), choose(0, 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, elseIfChainNoFinalElse) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int choose(int a, int b, int c) {
            if (a)
                return 1;
            else if (b)
                return 2;
            else if (c)
                return 3;
            return 4;
        }
        int main() {
            printf("%d %d %d %d", choose(1, 0, 0), choose(0, 1, 0),
                choose(0, 0, 1), choose(0, 0, 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}

TEST(Compiler, elseIfWithFinalElse) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int choose(int a, int b) {
            if (a)
                return 1;
            else if (b)
                return 2;
            else
                return 3;
        }
        int main() {
            printf("%d %d %d", choose(1, 0), choose(0, 1), choose(0, 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

// else binds to the inner if, not the outer (C dangling-else).
TEST(Compiler, danglingElseBindsToInnerIf) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int pick(int a, int b) {
            int r;
            r = 0;
            if (a)
                if (b)
                    r = 1;
                else
                    r = 2;
            return r;
        }
        int main() {
            printf("%d %d %d %d", pick(1, 1), pick(1, 0), pick(0, 1), pick(0, 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 0 0");
}

TEST(Compiler, elseIfInsideWhile) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int i;
            int n;
            i = 0;
            n = 0;
            while (i < 4) {
                if (i == 0)
                    n = n + 1;
                else if (i == 1)
                    n = n + 10;
                else if (i == 2)
                    n = n + 100;
                i = i + 1;
            }
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("111");
}

}
