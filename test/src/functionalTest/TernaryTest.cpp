#include "TestFixtures.h"

namespace {

TEST(Compiler, ternaryBasicTrue) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1 ? 10 : 20;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10");
}

TEST(Compiler, ternaryBasicFalse) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 0 ? 10 : 20;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, ternaryWithVariables) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            int y;
            int z;
            x = 5;
            y = 3;
            z = (x > y) ? x : y;
            printf("%d", z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, ternaryNested) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1 ? (0 ? 1 : 2) : 3;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, ternaryInExpression) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 2 + (1 ? 3 : 4);
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

// glibc/git assert expands to a void ternary: cond ? (void)0 : die(...).
TEST(Compiler, voidTernaryStatementLikeAssert) {
    SourceProgram program{R"prg(
        void die(const char *m) {
            printf("%s", m);
        }
        int main() {
            int n;
            n = 1;
            (n ? (void)0 : die("bad"));
            n = 0;
            (n ? (void)0 : die("x"));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("x");
}

// Comma + void ternary (closer to real assert expansion).
TEST(Compiler, voidTernaryWithCommaCondition) {
    SourceProgram program{R"prg(
        void die(const char *m) {
            printf("%s", m);
        }
        int main() {
            int n;
            n = 1;
            ((void)0, n ? (void)0 : die("no"));
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// C 6.3.2.1 / 6.5.15: array operands of ?: decay to pointer-to-first-element.
// Without decay, the ternary copies the array into a stack temporary and the
// "pointer" is the address of that temp - pointer equality with the global
// fails and free(setto) crashes (git attr: e->setto = c ? ATTR__FALSE : ATTR__UNSET).
TEST(Compiler, ternaryArrayOperandDecaysToPointer) {
    SourceProgram program{R"prg(
        const char A[] = "aaa";
        const char B[] = "bbb";
        int main() {
            const char *p;
            int c;
            c = 1;
            p = c ? A : B;
            printf("%d %d %c ", p == A, p == B, *p);
            c = 0;
            p = c ? A : B;
            printf("%d %d %c", p == A, p == B, *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 a 0 1 b");
}

// git attr parse_attr: e->setto = (*cp == '-') ? ATTR__FALSE : ATTR__UNSET;
// ATTR__FALSE is a global char array; ATTR__UNSET is a null pointer constant.
TEST(Compiler, ternaryGlobalArrayVsNullPointerSentinel) {
    SourceProgram program{R"prg(
        const char git_attr__false[] = "\0(builtin)false";
        const char git_attr__true[] = "(builtin)true";
        void free_if_heap(const char *setto) {
            if (setto == git_attr__true || setto == git_attr__false || setto == 0)
                printf("skip ");
            else
                printf("free ");
        }
        int main() {
            const char *setto;
            int c;
            c = '-';
            setto = (c == '-') ? git_attr__false : 0;
            free_if_heap(setto);
            printf("%d ", setto == git_attr__false);
            c = '!';
            setto = (c == '-') ? git_attr__false : 0;
            free_if_heap(setto);
            printf("%d", setto == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("skip 1 skip 1");
}

// git xdiff get_func_line: buf = func_line ? func_line->buf : dummy;
// Member arrays keep expression type T[N] for sizeof, but the result symbol is
// already &member[0]. Ternary must use that pointer type for its result; using
// getType() yields an array temporary and assignment takes &stack_temp, so
// memcpy never writes the real buffer (hunk header funcname becomes NUL).
TEST(Compiler, ternaryMemberArrayDecaysToPointer) {
    SourceProgram program{R"prg(
        void *memcpy(void *d, const void *s, unsigned long n);
        struct func_line {
            long len;
            char buf[80];
        };
        int main() {
            struct func_line fl;
            struct func_line *p;
            char *buf;
            char dummy[1];
            int i;
            fl.len = 0;
            for (i = 0; i < 80; i = i + 1) {
                fl.buf[i] = 0;
            }
            p = &fl;
            buf = p ? p->buf : dummy;
            memcpy(buf, "B", 1);
            printf("%d %c ", (int)fl.buf[0], fl.buf[0] ? fl.buf[0] : '?');
            /* Dot form of the same pattern */
            fl.buf[0] = 0;
            buf = 1 ? fl.buf : dummy;
            memcpy(buf, "C", 1);
            printf("%d %c", (int)fl.buf[0], fl.buf[0] ? fl.buf[0] : '?');
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("66 B 67 C");
}

} // namespace
