#include "TestFixtures.h"

namespace {

TEST(Compiler, canPassAndOutputArguments) {
    SourceProgram program{R"prg(
        void function(int a, int b) {
            output a;
            output b;
        }

        int main() {
            int a, b;
            input a;
            input b;
            function(a, b);
            return 0;
        }
    )prg"};

    program.compile();

    std::string input {"1\n2"};
    program.runAndExpect(input, input +"\n");
}

TEST(Compiler, canPassAndOutputManyArguments) {
    SourceProgram program{R"prg(
        void function(int a, int b, int c, int d, int e, int f, int g,
                      int h, int i, int j, int k, int l, int m, int n,
                      int o, int p, int q, int r, int s, int t, int u,
                      int v, int w, int x, int y, int z) {
            output a;
            output b;
            output c;
            output d;
            output e;
            output f;
            output g;
            output h;
            output i;
            output j;
            output k;
            output l;
            output m;
            output n;
            output o;
            output p;
            output q;
            output r;
            output s;
            output t;
            output u;
            output v;
            output w;
            output x;
            output y;
            output z;
        }

        int main() {
            int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
            input a;
            input b;
            input c;
            input d;
            input e;
            input f;
            input g;
            input h;
            input i;
            input j;
            input k;
            input l;
            input m;
            input n;
            input o;
            input p;
            input q;
            input r;
            input s;
            input t;
            input u;
            input v;
            input w;
            input x;
            input y;
            input z;
            function(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
            return 0;
        }
    )prg"};

    program.compile();

    std::string input {"1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26"};
    program.runAndExpect(input, input + "\n");
}

}
