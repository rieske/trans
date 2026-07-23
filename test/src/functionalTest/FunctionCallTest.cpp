#include "TestFixtures.h"

namespace {

TEST(Compiler, canPassAndOutputArguments) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
        int main() {
            printf("%d %d %d %d %d %d %d\n", 1, 2, 3, 4, 5, 6, 7);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 2 3 4 5 6 7\n");
}

// Register args (especially arg4/rcx) must not be clobbered when pushing stack args
// that require loading a global (lea scratch). git commit_tree_extended hit this:
// parents in rcx was overwritten by lea rcx, [rel sign_commit] while setting up
// stack args author/committer/extra.
TEST(Compiler, stackArgSetupDoesNotClobberRegisterArgs) {
    SourceProgram program{R"prg(
        int g_stack = 70;
        char *g_ptr = 0;

        void nine_args(int a, int b, int c, int *p, int e, int f,
                       char *s, int h, int i) {
            printf("%d %d %d %d %d %d %d %d\n", a, b, c, *p, e, f, h, i);
        }

        int main() {
            int x = 40;
            nine_args(10, 20, 30, &x, 50, 60, g_ptr, g_stack, 80);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10 20 30 40 50 60 70 80\n");
}

TEST(Compiler, canPassAndOutputManyArguments) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
        int main() {
            printf("%d %d %d %d %d %d %d %d\n", 1, 2, 3, 4, 5, 6, 7, 8);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4 5 6 7 8\n");
}

TEST(Compiler, nestedFunctionCalls) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

// Many expression temps per frame used to reserve one stack slot each for the
// whole function (~3KB frames). Recursion then SEGVs near depth ~2800 on an
// 8MB stack. Temp slots must be reused across statements so deep recursion works.
TEST(Compiler, deepRecursionWithManyExpressionTemps) {
    SourceProgram program{R"prg(
        int g;
        int work(int n, int a, int b, int c, int d) {
            int x;
            if (n <= 0) {
                return a;
            }
            x = a + b + c + d + n;
            x = x * 2 + (a - b) + (c - d);
            x = x + (a + 1) + (b + 2) + (c + 3) + (d + 4);
            x = x + (a + b) * (c + d);
            return work(n - 1, x, a, b, c);
        }

        int main() {
            // Depth 4000 must fit in a default 8MB stack after frame packing.
            g = work(4000, 1, 2, 3, 4);
            printf("%d", g != 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Consecutive statements each create temps; reuse must not clobber earlier results
// that were already stored to named locals.
TEST(Compiler, tempSlotReuseAcrossStatementsPreservesLocals) {
    SourceProgram program{R"prg(
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

// Declared but not defined in this TU - must emit `extern strlen` and link with libc.
TEST(Compiler, callsExternalLibraryFunction) {
    SourceProgram program{R"prg(
        int strlen(const char *s);

        int main() {
            printf("%d", strlen("hello"));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

// Taking the address of an external function also requires `extern`.
TEST(Compiler, takesAddressOfExternalFunction) {
    SourceProgram program{R"prg(
        int strlen(const char *s);

        int main() {
            int (*fp)(const char *);
            fp = strlen;
            printf("%d", fp("ab"));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

// Indirect call with 4 register args: the call target must not live in rcx
// (the 4th arg register). git config_fn_t is (var, value, ctx, data).
TEST(Compiler, functionPointerFourArgs) {
    SourceProgram program{R"prg(
        int add4(int a, int b, int c, int d) {
            return a + b + c + d;
        }

        int main() {
            int (*fp)(int, int, int, int);
            fp = add4;
            printf("%d", fp(1, 2, 3, 4));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10");
}

// Same pattern with a pointer "data" 4th arg (git config_include_data *).
TEST(Compiler, functionPointerFourArgsWithDataPointer) {
    SourceProgram program{R"prg(
        struct Data {
            int x;
        };

        int use(int a, int b, int c, struct Data *d) {
            return a + b + c + d->x;
        }

        int main() {
            struct Data d;
            int (*fp)(int, int, int, struct Data *);
            d.x = 40;
            fp = use;
            printf("%d", fp(1, 2, 3, &d));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("46");
}

}
