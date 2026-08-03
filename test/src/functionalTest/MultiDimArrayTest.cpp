#include "TestFixtures.h"

namespace {

TEST(Compiler, multiDimensionalArrayAccess) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[2][3];
            a[0][0] = 1;
            a[0][1] = 2;
            a[0][2] = 3;
            a[1][0] = 4;
            a[1][1] = 5;
            a[1][2] = 6;
            printf("%d %d %d %d", a[0][0], a[0][2], a[1][0], a[1][2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3 4 6");
}

TEST(Compiler, multiDimensionalCharArrayIndex) {
    SourceProgram program{R"prg(#include <stdio.h>
        static char topath[4][8];
        int main() {
            int i;
            topath[1][0] = 65;
            topath[2][0] = 66;
            i = 1;
            if (topath[i][0]) {
                printf("%c", topath[i][0]);
            }
            i = 2;
            printf("%c", topath[i][0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("AB");
}

// Multi-dim row decay (C 6.3.2.1): a[i] has type T[N] for sizeof but decays to
// T* as a value — not T(*)[N]. Required for git topath[i] as char*, row + k, etc.
TEST(Compiler, multiDimensionalRowDecaysToElementPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[2][3];
            int *p;
            a[0][0] = 1;
            a[0][1] = 2;
            a[0][2] = 3;
            a[1][0] = 4;
            a[1][1] = 5;
            a[1][2] = 6;
            p = a[0];
            printf("%d %d %d", p[0], p[1], p[2]);
            p = a[1];
            printf(" %d %d %d", p[0], p[1], p[2]);
            return 0;
        }
    )prg", "multi_dim_row_decay"};
    program.compile();
    program.runAndExpect("1 2 3 4 5 6");
}

// a[i] + k scales by sizeof(T), not sizeof(T[N]) (T* not T(*)[N]).
TEST(Compiler, multiDimensionalRowPointerArithmetic) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[2][3];
            a[0][0] = 10;
            a[0][1] = 20;
            a[0][2] = 30;
            a[1][0] = 40;
            a[1][1] = 50;
            a[1][2] = 60;
            printf("%d %d %d", *(a[0] + 1), *(a[0] + 2), *(a[1] + 1));
            return 0;
        }
    )prg", "multi_dim_row_arith"};
    program.compile();
    program.runAndExpect("20 30 50");
}

// Pass multi-dim row as T* argument (decay at call).
TEST(Compiler, multiDimensionalRowAsPointerArg) {
    SourceProgram program{R"prg(#include <stdio.h>
        void show(int *p) {
            printf("%d %d %d", p[0], p[1], p[2]);
        }
        int main() {
            int a[2][3];
            a[1][0] = 7;
            a[1][1] = 8;
            a[1][2] = 9;
            show(a[1]);
            return 0;
        }
    )prg", "multi_dim_row_arg"};
    program.compile();
    program.runAndExpect("7 8 9");
}

// sizeof(a[i]) is the row size N*sizeof(T), not sizeof(T*).
TEST(Compiler, multiDimensionalRowSizeofIsArray) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[2][3];
            char c[4][8];
            printf("%d %d", (int)sizeof(a[0]), (int)sizeof(c[1]));
            return 0;
        }
    )prg", "multi_dim_row_sizeof"};
    program.compile();
    program.runAndExpect("12 8");
}

// git-shaped: char topath[N][M]; char *p = topath[i]; p[j] = ...
TEST(Compiler, multiDimensionalCharRowAsCharPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        static char topath[4][8];
        int main() {
            char *p;
            topath[2][0] = 120;
            topath[2][1] = 121;
            topath[2][2] = 0;
            p = topath[2];
            printf("%s %d", p, (int)p[1]);
            return 0;
        }
    )prg", "multi_dim_char_row"};
    program.compile();
    program.runAndExpect("xy 121");
}

// C 6.7.9: a nested braced string initializes the row, not the first char.
TEST(Compiler, multiDimensionalCharRowFromNestedBracedString) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char a[2][8] = { { "ab" }, { "cd" } };
            printf("%s %s %d %d", a[0], a[1], (int)a[0][0], (int)a[1][0]);
            return 0;
        }
    )prg", "multi_dim_nested_brace_str"};
    program.compile();
    program.runAndExpect("ab cd 97 99");
}

TEST(Compiler, staticMultiDimensionalCharRowFromNestedBracedString) {
    SourceProgram program{R"prg(#include <stdio.h>
        static char a[2][8] = { { "ab" }, { "cd" } };
        int main() {
            printf("%s %s %d %d", a[0], a[1], (int)a[0][0], (int)a[1][0]);
            return 0;
        }
    )prg", "static_multi_dim_nested_brace_str"};
    program.compile();
    program.runAndExpect("ab cd 97 99");
}

} // namespace
