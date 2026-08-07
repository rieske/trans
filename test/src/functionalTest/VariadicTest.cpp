#include "TestFixtures.h"

namespace {

TEST(Compiler, vaListSizeIsTwentyFour) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", (int)sizeof(__builtin_va_list));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("24");
}

TEST(Compiler, variadicOneExtraArg) {
    SourceProgram program{R"prg(
        int first_extra(int x, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, x);
            int y = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return y;
        }

        int main() {
            printf("%d", first_extra(10, 42));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, variadicSumTwoExtraArgs) {
    SourceProgram program{R"prg(
        int add2(int base, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, base);
            int a = __builtin_va_arg(ap, int);
            int b = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return base + a + b;
        }

        int main() {
            printf("%d", add2(1, 2, 3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, vaStartGpOffsetSkipsNamedArg) {
    SourceProgram program{R"prg(
        int first_and_offset(int named, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, named);
            int gp = (int)ap[0].gp_offset;
            int first = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return gp * 1000 + first;
        }

        int main() {
            printf("%d", first_and_offset(7, 42));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8042");
}

TEST(Compiler, vaArgDoubleDirect) {
    SourceProgram program{R"prg(
        int first_double(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, n);
            double d = __builtin_va_arg(ap, double);
            __builtin_va_end(ap);
            return (int)(d * 10.0);
        }
        int main() {
            printf("%d", first_double(0, 4.2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, vaArgIntegerOverflowArea) {
    SourceProgram program{R"prg(
        int sum_n(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, n);
            int s = 0;
            int i;
            for (i = 0; i < n; i = i + 1) {
                s = s + __builtin_va_arg(ap, int);
            }
            __builtin_va_end(ap);
            return s;
        }
        int main() {
            printf("%d", sum_n(8, 1, 2, 3, 4, 5, 6, 7, 8));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("36");
}

TEST(Compiler, vaArgDoubleOverflowArea) {
    SourceProgram program{R"prg(
        int last_double(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, n);
            double d = 0.0;
            int i;
            for (i = 0; i < n; i = i + 1) {
                d = __builtin_va_arg(ap, double);
            }
            __builtin_va_end(ap);
            return (int)d;
        }
        int main() {
            printf("%d", last_double(8,
                1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 42.0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, vaListParameterDecaysToPointer) {
    SourceProgram program{R"prg(
        int take_ap(__builtin_va_list ap) {
            return __builtin_va_arg(ap, int);
        }
        int wrap(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, n);
            int v = take_ap(ap);
            __builtin_va_end(ap);
            return v;
        }
        int main() {
            printf("%d", wrap(0, 99));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("99");
}

TEST(Compiler, variadicVaCopy) {
    SourceProgram program{R"prg(
        int sum_twice(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_list cp;
            __builtin_va_start(ap, n);
            __builtin_va_copy(cp, ap);
            int a = __builtin_va_arg(ap, int);
            int b = __builtin_va_arg(cp, int);
            __builtin_va_end(ap);
            __builtin_va_end(cp);
            return a + b;
        }

        int main() {
            printf("%d", sum_twice(1, 21));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, variadicNestedCallPreservesOuterStackVarargs) {
    SourceProgram program{R"prg(
        void inner_fmt(char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            (void)__builtin_va_arg(ap, char *);
            __builtin_va_end(ap);
        }
        int outer_hooks(int a, char *b, int *c, char *name, ...) {
            char sink;
            (void)a;
            (void)b;
            (void)c;
            (void)sink;
            inner_fmt("%s", "nested");
            __builtin_va_list ap;
            __builtin_va_start(ap, name);
            int count = 0;
            char *s;
            while ((s = __builtin_va_arg(ap, char *)) != 0) {
                count = count + 1;
                sink = s[0];
            }
            __builtin_va_end(ap);
            return count;
        }
        int main() {
            int n = outer_hooks(1, "idx", 0, "prepare-commit-msg",
                                "editmsg", "arg1", "arg2", 0);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, c23VaStartReadsFirstVararg) {
    SourceProgram program{R"prg(
        int first_c23(int named, ...) {
            __builtin_va_list ap;
            __builtin_c23_va_start(ap, named);
            int first = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return first;
        }

        int main() {
            printf("%d", first_c23(7, 42));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, c23VaStartSingleArgForm) {
    SourceProgram program{R"prg(
        int first_slot(int named, ...) {
            __builtin_va_list ap;
            __builtin_c23_va_start(ap);
            int v = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return v + named;
        }
        int main() {
            printf("%d", first_slot(10, 32));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, vaArgPointerAndLong) {
    SourceProgram program{R"prg(
        long take_ptr_and_long(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, n);
            int *p = __builtin_va_arg(ap, int *);
            long l = __builtin_va_arg(ap, long);
            __builtin_va_end(ap);
            return (long)(*p) + l;
        }
        int main() {
            int x = 20;
            printf("%d", (int)take_ptr_and_long(0, &x, 22L));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, vaArgOfStructIsAcceptedByFrontend) {
    SourceProgram program{R"prg(
        struct Pair {
            long a;
            long b;
        };
        long first_field(int n, ...) {
            __builtin_va_list ap;
            struct Pair p;
            __builtin_va_start(ap, n);
            p = __builtin_va_arg(ap, struct Pair);
            __builtin_va_end(ap);
            return p.a;
        }
        int main() {
            struct Pair q;
            q.a = 11;
            q.b = 22;
            (void)first_field(0, q);
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

} // namespace
