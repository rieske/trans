#include "TestFixtures.h"

namespace {

TEST(Compiler, ternaryBasicTrue) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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

// C 6.5.15: array arms decay; different lengths are still pointer-compatible.
TEST(Compiler, ternaryDecaysDifferentLengthStringArrays) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            const char *p = 1 ? "short" : "much longer string";
            const char *q = 0 ? "short" : "much longer string";
            printf("%s %s", p, q);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("short much longer string");
}

// Git usage tables: const char *const usage_a[] vs usage_b[] of different N.
TEST(Compiler, ternaryDecaysDifferentLengthPointerArrays) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static const char *const usage_a[] = { "a", "b", 0 };
        static const char *const usage_b[] = { "x", 0 };
        int main(void) {
            const char *const *u = 1 ? usage_a : usage_b;
            const char *const *v = 0 ? usage_a : usage_b;
            printf("%s %s", u[0], v[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("a x");
}

// Git worktree shape: argv (pointer) vs compound array arm.
TEST(Compiler, ternaryPointerVsArrayOfPointers) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(int argc, const char **argv) {
            const char *self[] = { "self", 0 };
            const char **p = argc > 1 ? argv : self;
            printf("%s", p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("self");
}

// Function designators in ternary arms decay to pointers.
TEST(Compiler, ternaryFunctionDesignatorArms) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int one(void) { return 1; }
        static int two(void) { return 2; }
        int main(void) {
            int (*fp)(void) = 1 ? one : two;
            printf("%d", fp());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Git oidmap shape: cond ? free : NULL is a function pointer, not void*.
TEST(Compiler, ternaryFunctionPointerVsNull) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static void freer(void *p) { (void)p; }
        int main(void) {
            void (*fp)(void *) = 1 ? freer : 0;
            void (*gp)(void *) = 0 ? freer : 0;
            printf("%d %d", fp != 0, gp == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

// Git hook shape: struct* && function-pointer as logical scalars.
TEST(Compiler, logicalAndStructPointerAndFunctionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct O {
            void *(*alloc)(void *);
            void *ctx;
        };
        static void *my_alloc(void *p) { return p; }
        int main(void) {
            struct O o;
            int x;
            o.alloc = my_alloc;
            o.ctx = &x;
            if (&o && o.alloc) {
                void *p = o.alloc(o.ctx);
                printf("%d", p == &x);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

} // namespace
