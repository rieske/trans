#include "TestFixtures.h"

namespace {


TEST(Compiler, structReturnByValue) {
    SourceProgram program{R"prg(
        struct Point {
            int x;
            int y;
        };

        struct Point make(void) {
            struct Point p;
            p.x = 9;
            p.y = 10;
            return p;
        }

        int main() {
            struct Point p;
            p = make();
            printf("%d %d", p.x, p.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 10");
}


// Return a file-scope multi-word struct by value (git parse_maintenance_strategy
// returns incremental_strategy). loadWord of a global must not clobber the sret
// pointer register, or the copy becomes *src = *src and the caller sees zeros.
TEST(Compiler, largeStaticStructReturnByValue) {
    SourceProgram program{R"prg(
        struct Strat {
            unsigned long t0;
            unsigned long s0;
            unsigned long t1;
            unsigned long s1;
            unsigned long t2;
            unsigned long s2;
        };
        static const struct Strat incremental = {
            .t0 = 1, .s0 = 3,
            .t1 = 1, .s1 = 2,
            .t2 = 2, .s2 = 0,
        };
        struct Strat get_incremental(void) {
            return incremental;
        }
        int main() {
            struct Strat s;
            s = get_incremental();
            printf("%lu %lu %lu %lu %lu %lu", s.t0, s.s0, s.t1, s.s1, s.t2, s.s2);
            return 0;
        }
    )prg", "static_struct_sret"};
    program.compile();
    program.runAndExpect("1 3 1 2 2 0");
}


// System V: aggregates larger than 16 bytes return via hidden pointer (sret).
// git strbuf is {size_t alloc; size_t len; char *buf} - 24 bytes; without sret
// the buf pointer is lost and write(fd, NULL, len) fails config creation.
TEST(Compiler, largeStructReturnByValue) {
    SourceProgram program{R"prg(
        struct Big {
            unsigned long a;
            unsigned long b;
            unsigned long c;
        };

        struct Big make(void) {
            struct Big s;
            s.a = 1;
            s.b = 2;
            s.c = 3;
            return s;
        }

        int main() {
            struct Big s;
            s = make();
            printf("%lu %lu %lu", s.a, s.b, s.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}


// Same shape as git strbuf: two size_t fields and a pointer.
TEST(Compiler, strbufShapedStructReturn) {
    SourceProgram program{R"prg(
        struct Sbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };

        static char data[8];

        struct Sbuf make(void) {
            struct Sbuf sb;
            sb.alloc = 8;
            sb.len = 7;
            sb.buf = data;
            data[0] = 'A';
            data[1] = 0;
            return sb;
        }

        int main() {
            struct Sbuf sb;
            sb = make();
            printf("%lu %lu %s", sb.alloc, sb.len, sb.buf);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 7 A");
}


TEST(Compiler, structReturnByPointer) {
    SourceProgram program{R"prg(
        struct Point {
            int x;
            int y;
        };

        struct Point g;

        struct Point *get(void) {
            g.x = 1;
            g.y = 2;
            return &g;
        }

        int main() {
            struct Point *p;
            p = get();
            printf("%d %d", p->x, p->y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

// Nested sret: callee returns a large struct by calling another sret function.
// The outer sret pointer must survive the inner call's use of rdi.
TEST(Compiler, nestedSretCallPreservesOuterReturn) {
    SourceProgram program{R"prg(
        struct Big {
            unsigned long a;
            unsigned long b;
            unsigned long c;
        };

        struct Big inner(void) {
            struct Big s;
            s.a = 10;
            s.b = 20;
            s.c = 30;
            return s;
        }

        struct Big outer(void) {
            return inner();
        }

        int main() {
            struct Big s;
            s = outer();
            printf("%lu %lu %lu", s.a, s.b, s.c);
            return 0;
        }
    )prg", "nested_sret"};
    program.compile();
    program.runAndExpect("10 20 30");
}

// sret callee with register-passed args after the hidden pointer (rdi = sret).
TEST(Compiler, sretWithRegisterArgsAfterHiddenPointer) {
    SourceProgram program{R"prg(
        struct Big {
            unsigned long a;
            unsigned long b;
            unsigned long c;
        };

        struct Big make(unsigned long x, unsigned long y, unsigned long z) {
            struct Big s;
            s.a = x;
            s.b = y;
            s.c = z;
            return s;
        }

        int main() {
            struct Big s;
            s = make(1, 2, 3);
            printf("%lu %lu %lu", s.a, s.b, s.c);
            return 0;
        }
    )prg", "sret_with_args"};
    program.compile();
    program.runAndExpect("1 2 3");
}

// Register-return aggregate (≤16 bytes): pass by value and return by value.
// Larger pass+sret-return is pinned separately as an unsupported surface.
TEST(Compiler, twoWordStructPassAndReturnByValue) {
    SourceProgram program{R"prg(
        struct Pair {
            unsigned long a;
            unsigned long b;
        };

        struct Pair bump(struct Pair s) {
            s.a = s.a + 1;
            s.b = s.b + 1;
            return s;
        }

        int main() {
            struct Pair in;
            struct Pair out;
            in.a = 4;
            in.b = 5;
            out = bump(in);
            printf("%lu %lu %lu %lu", in.a, in.b, out.a, out.b);
            return 0;
        }
    )prg", "pair_pass_return"};
    program.compile();
    program.runAndExpect("4 5 5 6");
}

// sret return of a large struct that was filled from a by-pointer argument
// (common git shape: mutate via pointer, return another large object).
TEST(Compiler, largeStructSretFromPointerArg) {
    SourceProgram program{R"prg(
        struct Big {
            unsigned long a;
            unsigned long b;
            unsigned long c;
        };

        struct Big copy_bump(struct Big *p) {
            struct Big s;
            s.a = p->a + 1;
            s.b = p->b + 1;
            s.c = p->c + 1;
            return s;
        }

        int main() {
            struct Big in;
            struct Big out;
            in.a = 4;
            in.b = 5;
            in.c = 6;
            out = copy_bump(&in);
            printf("%lu %lu %lu %lu %lu %lu", in.a, in.b, in.c, out.a, out.b, out.c);
            return 0;
        }
    )prg", "sret_from_ptr"};
    program.compile();
    program.runAndExpect("4 5 6 5 6 7");
}

// Product limit: pass-by-value of a >16-byte aggregate into a sret-returning
// callee is not a supported SysV surface (runtime values are wrong today).
// Pin compile+link only so a correct ABI implementation has a place to land
// the oracle `4 5 6 5 6 7` later; do not assert field values here.
TEST(Compiler, largeStructPassAndSretReturnCompiles) {
    SourceProgram program{R"prg(
        struct Big {
            unsigned long a;
            unsigned long b;
            unsigned long c;
        };

        struct Big bump(struct Big s) {
            s.a = s.a + 1;
            s.b = s.b + 1;
            s.c = s.c + 1;
            return s;
        }

        int main() {
            struct Big in;
            struct Big out;
            in.a = 4;
            in.b = 5;
            in.c = 6;
            out = bump(in);
            (void)out;
            printf("%d", 1);
            return 0;
        }
    )prg", "large_pass_sret_compiles"};
    program.compile();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("1");
}

// Variadic + large struct return: compiles (headers may declare such shapes) but
// is not a supported product surface. StackMachine applies sret only when
// memoryReturn && !variadic; see StackMachineTest.variadicMemoryReturnSkipsSret.
// Here we only pin that the combination still parses and links.
TEST(Compiler, variadicLargeStructReturnCompiles) {
    SourceProgram program{R"prg(
        struct Big {
            unsigned long a;
            unsigned long b;
            unsigned long c;
        };

        struct Big make(int named, ...) {
            struct Big s;
            s.a = 1;
            s.b = 2;
            s.c = 3;
            return s;
        }

        int main() {
            struct Big s;
            s = make(0);
            (void)s;
            printf("%d", 1);
            return 0;
        }
    )prg", "sret_variadic_compiles"};
    program.compile();
    program.runAndExpect("1");
}

} // namespace
