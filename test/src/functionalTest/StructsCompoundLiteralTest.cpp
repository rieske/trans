#include "TestFixtures.h"

namespace {



// C99 compound literal assigned to a struct (git push: rs = (struct refspec){...}).
TEST(Compiler, compoundLiteralAssignment) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
        };
        int main() {
            struct S s;
            s.a = 1;
            s.b = 2;
            s = (struct S) { .a = 7 };
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg", "cl_assign"};
    program.compileWithPreprocess();
    program.runAndExpect("7 0");
}


// glibc byteswap headers look like:
//   static __uint16_t __bswap_16 (__uint16_t __bsx) { return ...; }
// Compound-literal parsing must not treat the parameter list
// (__uint16_t __bsx){ as a compound literal (two idents => declarator, not type).
TEST(Compiler, functionParamNotCompoundLiteral) {
    SourceProgram program{R"prg(
        static unsigned short bswap16(unsigned short x) {
            return (unsigned short)(((x >> 8) & 0xff) | ((x & 0xff) << 8));
        }
        int main() {
            printf("%d", (int)bswap16(0x1234));
            return 0;
        }
    )prg", "fn_param_not_cl"};
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("13330");
}


// Empty parameter list (void) immediately before a function body must not be
// rewritten as a compound literal: static int f(void) { return 1; }
TEST(Compiler, voidParamListNotCompoundLiteral) {
    SourceProgram program{R"prg(
        static int have_feature(void) {
            return 1;
        }
        int main() {
            printf("%d", have_feature());
            return 0;
        }
    )prg", "void_param_not_cl"};
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("1");
}


// git date.c:get_time - if (x) { ... } else ... must not be rewritten as a
// compound literal of "type" x (bare id is a variable, not a typedef).
TEST(Compiler, ifConditionNotCompoundLiteral) {
    SourceProgram program{R"prg(
        int main() {
            const char *x;
            int sec;
            int usec;
            x = "42";
            sec = 0;
            usec = 1;
            if (x) {
                sec = 42;
                usec = 0;
            } else {
                sec = 99;
                usec = 99;
            }
            printf("%d %d", sec, usec);
            return 0;
        }
    )prg", "if_cond_not_cl"};
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("42 0");
}


// git date.c:approxidate - switch (c) { case ... } must not treat (c){ as CL.
TEST(Compiler, switchConditionNotCompoundLiteral) {
    SourceProgram program{R"prg(
        int classify(int c) {
            switch (c) {
            case 1:
                return 10;
            case 2:
                return 20;
            default:
                return 0;
            }
        }
        int main() {
            printf("%d %d %d", classify(1), classify(2), classify(3));
            return 0;
        }
    )prg", "switch_cond_not_cl"};
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("10 20 0");
}


// while (n) { ... } is the same class of false positive as if (x) {.
TEST(Compiler, whileConditionNotCompoundLiteral) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int s;
            n = 3;
            s = 0;
            while (n) {
                s = s + n;
                n = n - 1;
            }
            printf("%d", s);
            return 0;
        }
    )prg", "while_cond_not_cl"};
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("6");
}


// git unit-tests/u-oid-array.c: TEST_LOOKUP(((const char *[]){ ... }), ...)
// expands to a call + ARRAY_SIZE of the same compound literal. Must not strip
// the cast to bare braces (invalid C) and must keep a real object for sizeof.
TEST(Compiler, compoundLiteralAsFunctionArgument) {
    SourceProgram program{R"prg(
        #define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
        int sum(const int *a, unsigned long n) {
            unsigned long i;
            int s;
            s = 0;
            for (i = 0; i < n; i = i + 1) {
                s = s + a[i];
            }
            return s;
        }
        int main() {
            int x;
            x = 10;
            printf("%d", sum(((const int[]){ 1, x, 3 }), ARRAY_SIZE(((const int[]){ 1, x, 3 }))));
            return 0;
        }
    )prg", "cl_as_arg"};
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("14");
}


// git t/unit-tests/u-prio-queue.c: TEST_INPUT(((int []){...}), ((int []){...}))
// passes two different compound literals in one statement. Both must be hoisted;
// rewriting only the first leaves ((int []){...}) which the subset parser rejects.
TEST(Compiler, twoCompoundLiteralsInOneStatement) {
    SourceProgram program{R"prg(
        #define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
        int check(const int *a, unsigned long na, const int *b, unsigned long nb) {
            unsigned long i;
            int s;
            s = 0;
            for (i = 0; i < na; i = i + 1) {
                s = s + a[i];
            }
            for (i = 0; i < nb; i = i + 1) {
                s = s + b[i] * 10;
            }
            return s;
        }
        int main() {
            printf("%d", check(((int []){ 1, 2 }), ARRAY_SIZE(((int []){ 1, 2 })),
                               ((int []){ 3, 4 }), ARRAY_SIZE(((int []){ 3, 4 }))));
            return 0;
        }
    )prg", "two_cl_one_stmt"};
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    // 1+2 + 10*(3+4) = 3 + 70 = 73
    program.runAndExpect("73");
}


// Compound literal as a parenthesized initializer (git REFTABLE_*_INIT macros).
TEST(Compiler, compoundLiteralAsInitializer) {
    SourceProgram program{R"prg(
        struct S {
            int fd;
            int x;
        };
        int main() {
            struct S t = ((struct S) { .fd = -1, });
            printf("%d %d", t.fd, t.x);
            return 0;
        }
    )prg", "cl_as_init"};
    program.compileWithPreprocess();
    program.runAndExpect("-1 0");
}


// Assignment of parenthesized compound literal through a pointer
// (git reftable: *t = REFTABLE_TMPFILE_INIT).
TEST(Compiler, compoundLiteralStarAssign) {
    SourceProgram program{R"prg(
        struct S {
            int fd;
            int x;
        };
        int main() {
            struct S t;
            struct S *p;
            t.fd = 0;
            t.x = 5;
            p = &t;
            *p = ((struct S) { .fd = -1, });
            printf("%d %d", t.fd, t.x);
            return 0;
        }
    )prg", "cl_star_assign"};
    program.compileWithPreprocess();
    program.runAndExpect("-1 0");
}


// git reftable/stack: table_locks[i] = REFTABLE_FLOCK_INIT (compound literal).
TEST(Compiler, compoundLiteralArrayIndexAssign) {
    SourceProgram program{R"prg(
        struct S {
            int fd;
            int x;
        };
        int main() {
            struct S locks[2];
            int i;
            locks[0].fd = 0;
            locks[0].x = 5;
            locks[1].fd = 0;
            locks[1].x = 5;
            i = 0;
            locks[i] = ((struct S) { .fd = -1, });
            i = 1;
            locks[i] = ((struct S) { .fd = -2, .x = 9 });
            printf("%d %d %d %d", locks[0].fd, locks[0].x, locks[1].fd, locks[1].x);
            return 0;
        }
    )prg", "cl_arr_idx_assign"};
    program.compileWithPreprocess();
    program.runAndExpect("-1 0 -2 9");
}

} // namespace
