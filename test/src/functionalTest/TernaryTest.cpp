#include "TestFixtures.h"

namespace {

// Proves conditionalExpression is required: master throws until implemented.
TEST(Compiler, ternaryBasicTrue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, ternaryOnlyOneArmEvaluated) {
    // Side effects: only the selected arm must run (via assignment to out).
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int out;
            int unused;
            out = 0;
            unused = 1 ? (out = 7) : (out = 9);
            printf("%d %d", out, unused);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 7");
}

// Global init folds constant ternary via evaluateConstant.
TEST(Compiler, ternaryConstantGlobalInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int g = 1 ? 4 : 5;

        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

// Selected arm's false path must not execute either.
TEST(Compiler, ternaryFalseArmOnly) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int out;
            int unused;
            out = 0;
            unused = 0 ? (out = 7) : (out = 9);
            printf("%d %d", out, unused);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 9");
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
