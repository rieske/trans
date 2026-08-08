#include "TestFixtures.h"

namespace {

TEST(Compiler, structReturnByValue) {
    SourceProgram program{R"prg(#include <stdio.h>
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

TEST(Compiler, structPassByValue) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        int sum(struct Point p) {
            return p.x + p.y;
        }

        int main() {
            struct Point p;
            p.x = 3;
            p.y = 4;
            printf("%d", sum(p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, largeStaticStructReturnByValue) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    )prg"};
    program.compile();
    program.runAndExpect("1 3 1 2 2 0");
}

TEST(Compiler, largeStructReturnByValue) {
    SourceProgram program{R"prg(#include <stdio.h>
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

TEST(Compiler, strbufShapedStructReturn) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Sbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };

        static char data;

        struct Sbuf make(void) {
            struct Sbuf sb;
            sb.alloc = 8;
            sb.len = 7;
            sb.buf = &data;
            data = 'A';
            return sb;
        }

        int main() {
            struct Sbuf sb;
            sb = make();
            printf("%lu %lu %d", sb.alloc, sb.len, sb.buf == &data);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 7 1");
}

TEST(Compiler, nestedSretCallPreservesOuterReturn) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    )prg"};
    program.compile();
    program.runAndExpect("10 20 30");
}

TEST(Compiler, sretWithRegisterArgsAfterHiddenPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, twoWordStructPassAndReturnByValue) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    )prg"};
    program.compile();
    program.runAndExpect("4 5 5 6");
}

TEST(Compiler, largeStructSretFromPointerArg) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    )prg"};
    program.compile();
    program.runAndExpect("4 5 6 5 6 7");
}

TEST(Compiler, largeStructPassAndSretReturnCompiles) {
    SourceProgram program{R"prg(#include <stdio.h>
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
            printf("%lu %lu %lu", out.a, out.b, out.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6 7");
}

} // namespace
