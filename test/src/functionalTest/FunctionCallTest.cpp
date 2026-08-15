#include "TestFixtures.h"

namespace {

TEST(Compiler, unsignedCharIndexUsesZeroExtend) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[256];
            unsigned char i;
            i = 255;
            a[255] = 42;
            printf("%d", a[i]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, canPassAndOutputArguments) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        void function(int a, int b) {
            printf("%d %d", a, b);
        }

        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            function(a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1\n2", "1 2");
}

// 7 total args: 6 in registers, 1 on the stack (odd stack-arg count must keep RSP 16-byte aligned)
TEST(Compiler, callWithSevenArguments) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d %d %d %d %d %d\n", 1, 2, 3, 4, 5, 6);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 2 3 4 5 6\n");
}

// 8 total args: 2 on the stack (even stack-arg count stays aligned without padding)
TEST(Compiler, callWithEightArguments) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d %d %d %d %d %d %d\n", 1, 2, 3, 4, 5, 6, 7);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 2 3 4 5 6 7\n");
}

TEST(Compiler, canPassAndOutputManyArguments) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        void function(int a, int b, int c, int d, int e, int f, int g,
                      int h, int i, int j, int k, int l, int m, int n,
                      int o, int p, int q, int r, int s, int t, int u,
                      int v, int w, int x, int y, int z) {
            printf("%d\n", a);
            printf("%d\n", b);
            printf("%d\n", c);
            printf("%d\n", d);
            printf("%d\n", e);
            printf("%d\n", f);
            printf("%d\n", g);
            printf("%d\n", h);
            printf("%d\n", i);
            printf("%d\n", j);
            printf("%d\n", k);
            printf("%d\n", l);
            printf("%d\n", m);
            printf("%d\n", n);
            printf("%d\n", o);
            printf("%d\n", p);
            printf("%d\n", q);
            printf("%d\n", r);
            printf("%d\n", s);
            printf("%d\n", t);
            printf("%d\n", u);
            printf("%d\n", v);
            printf("%d\n", w);
            printf("%d\n", x);
            printf("%d\n", y);
            printf("%d\n", z);
        }

        int main() {
            int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
            scanf("%ld", &a);
            scanf("%ld", &b);
            scanf("%ld", &c);
            scanf("%ld", &d);
            scanf("%ld", &e);
            scanf("%ld", &f);
            scanf("%ld", &g);
            scanf("%ld", &h);
            scanf("%ld", &i);
            scanf("%ld", &j);
            scanf("%ld", &k);
            scanf("%ld", &l);
            scanf("%ld", &m);
            scanf("%ld", &n);
            scanf("%ld", &o);
            scanf("%ld", &p);
            scanf("%ld", &q);
            scanf("%ld", &r);
            scanf("%ld", &s);
            scanf("%ld", &t);
            scanf("%ld", &u);
            scanf("%ld", &v);
            scanf("%ld", &w);
            scanf("%ld", &x);
            scanf("%ld", &y);
            scanf("%ld", &z);
            function(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
            return 0;
        }
    )prg"};

    program.compile();

    std::string input {"1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26"};
    program.runAndExpect(input, input + "\n");
}

TEST(Compiler, userFunctionSevenArgs) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int take7(int a, int b, int c, int d, int e, int x, int g) {
            printf("%d", g);
            return g;
        }

        int main() {
            take7(1, 2, 3, 4, 5, 6, 7);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, onlyStackFormalsUsed) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int take8(int a, int b, int c, int d, int e, int x, int g, int h) {
            printf("%d %d", g, h);
            return 0;
        }

        int main() {
            take8(1, 2, 3, 4, 5, 6, 7, 8);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8");
}

TEST(Compiler, callWithNineArguments) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d %d %d %d %d %d %d %d\n", 1, 2, 3, 4, 5, 6, 7, 8);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4 5 6 7 8\n");
}

TEST(Compiler, nestedFunctionCalls) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int inc(int x) {
            return x + 1;
        }

        int main() {
            printf("%d", inc(inc(3)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, manyArgsReturnSum) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int sum3(int a, int b, int c, int d, int e, int f, int g) {
            return a + b + c + d + e + f + g;
        }

        int main() {
            printf("%d", sum3(1, 2, 3, 4, 5, 6, 7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("28");
}

TEST(Compiler, recursiveCountdown) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int countdown(int n) {
            if (n) {
                return countdown(n - 1) + 1;
            } else {
                return 0;
            }
        }

        int main() {
            printf("%d", countdown(3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, variadicFunctionIgnoresExtraArgs) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int first(int n, ...) {
            return n;
        }

        int main() {
            printf("%d", first(7, 1, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, variadicPrototypeThenDefinition) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int first(int n, ...);

        int first(int n, ...) {
            return n;
        }

        int main() {
            printf("%d", first(9, 1, 2, 3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

// Unrolled statements each create distinct temps. Without slot reuse the
// frame is ~2KB and recursion SEGVs near depth 4000 on an 8MB stack.
TEST(Compiler, deepRecursionWithManyExpressionTemps) {
    std::string src = R"prg(int printf(const char *, ...);
        int g;
        int work(int n, int a, int b, int c, int d) {
            int x;
            if (n <= 0) {
                return a;
            }
    )prg";
    for (int i = 0; i < 20; ++i) {
        src += "            x = a + b + c + d + n + a + b + c + d + n;\n";
        src += "            x = x + a + b + c + d + n;\n";
    }
    src += R"prg(            return work(n - 1, x, a, b, c);
        }

        int main() {
            g = work(4000, 1, 2, 3, 4);
            printf("%d", g != 0);
            return 0;
        }
    )prg";
    SourceProgram program{src};
    program.compile();
    program.runAndExpect("1");
}

// Consecutive statements each create temps; reuse must not clobber named locals.
TEST(Compiler, tempSlotReuseAcrossStatementsPreservesLocals) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int b;
            int c;
            a = 1 + 2 + 3 + 4 + 5;
            b = 10 + 20 + 30 + 40 + 50;
            c = a * 2 + b * 3;
            printf("%d %d %d", a, b, c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("15 150 480");
}

}
