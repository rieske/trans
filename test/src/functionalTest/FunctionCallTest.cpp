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

// FIXME: segfaults when more outputs are replaced with printfs
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

}
