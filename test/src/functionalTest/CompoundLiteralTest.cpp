#include "TestFixtures.h"

namespace {

// C99 compound literals: ( type-name ) { initializer-list }
// Git shapes: REFSPEC_INIT_*, REFTABLE_*_INIT, *p = INIT, locks[i] = INIT.

TEST(Compiler, scalarCompoundLiteralInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            x = (int){ 42 };
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, scalarCompoundLiteralNullPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int *p;
            p = (int *){ 0 };
            printf("%d", !p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Same scalar-brace policy as local `int x = {1, 2};`.
TEST(Compiler, scalarCompoundLiteralExcessElementsIsError) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = (int){ 1, 2 };
            return x;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements in scalar initializer");
}

// Nested scalar braces unwrap like local `int x = {{7}};`.
TEST(Compiler, scalarCompoundLiteralNestedBrace) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            x = (int){ { 7 } };
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

// git push: rs = (struct refspec) REFSPEC_INIT_PUSH  ->  (struct S){ .a = ... }
TEST(Compiler, compoundLiteralStructAssignDesignated) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        int main() {
            struct S s;
            s.a = 1;
            s.b = 2;
            s = (struct S){ .a = 7 };
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 0");
}

// git REFTABLE_*_INIT: struct S t = ((struct S){ .fd = -1, });
TEST(Compiler, compoundLiteralAsInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int fd;
            int x;
        };
        int main() {
            struct S t = ((struct S){ .fd = -1, });
            printf("%d %d", t.fd, t.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 0");
}

// git reftable: *t = REFTABLE_TMPFILE_INIT
TEST(Compiler, compoundLiteralStarAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
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
            *p = ((struct S){ .fd = -1, });
            printf("%d %d", t.fd, t.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 0");
}

// git reftable/stack: table_locks[i] = REFTABLE_FLOCK_INIT
TEST(Compiler, compoundLiteralArrayIndexAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
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
            locks[i] = ((struct S){ .fd = -1, });
            i = 1;
            locks[i] = ((struct S){ .fd = -2, .x = 9 });
            printf("%d %d %d %d", locks[0].fd, locks[0].x, locks[1].fd, locks[1].x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 0 -2 9");
}

TEST(Compiler, compoundLiteralPositionalElements) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int x;
            int y;
        };
        int main() {
            struct S s = (struct S){ 7 };
            printf("%d %d", s.x, s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 0");
}

TEST(Compiler, compoundLiteralTrailingComma) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int x;
            int y;
        };
        int main() {
            struct S s = (struct S){ 3, 4, };
            printf("%d %d", s.x, s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

TEST(Compiler, compoundLiteralMemberAccess) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        int main() {
            printf("%d %d", ((struct S){ 1, 2 }).a, ((struct S){ 3, 4 }).b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 4");
}

// Address of compound literal (array form) as function argument.
TEST(Compiler, compoundLiteralAsFunctionArgument) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int sum3(const int *a) {
            return a[0] + a[1] + a[2];
        }
        int main() {
            int x;
            x = 10;
            printf("%d", sum3(((const int[]){ 1, x, 3 })));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("14");
}

// --- Parse guards: (type){ must not steal function params or if/while/switch bodies ---

// static T f(T x) { ... }  must not parse (T x){ as a compound literal.
TEST(Compiler, functionParamListNotCompoundLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static unsigned short bswap16(unsigned short x) {
            return (unsigned short)(((x >> 8) & 0xff) | ((x & 0xff) << 8));
        }
        int main() {
            printf("%d", (int)bswap16(0x1234));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("13330");
}

TEST(Compiler, voidParamListNotCompoundLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int have_feature(void) {
            return 1;
        }
        int main() {
            printf("%d", have_feature());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, ifConditionNotCompoundLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            const char *x;
            int sec;
            x = "42";
            sec = 0;
            if (x) {
                sec = 42;
            } else {
                sec = 99;
            }
            printf("%d", sec);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, whileConditionNotCompoundLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
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
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, switchConditionNotCompoundLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
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
    )prg"};
    program.compile();
    program.runAndExpect("10 20 0");
}

TEST(Compiler, fileScopeCompoundLiteralArrayPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int *p = (int []){ 1, 2, 3 };
        int main() {
            printf("%d %d %d", p[0], p[1], p[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, fileScopeCompoundLiteralPlusInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int *p = (int []){ 1, 2, 3 } + 1;
        int main() {
            printf("%d %d", p[0], p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 3");
}

TEST(Compiler, fileScopeCompoundLiteralIndexAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int *p = &(int []){ 1, 2, 3 }[1];
        int main() {
            printf("%d %d", p[0], p[-1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

TEST(Compiler, fileScopeCompoundLiteralScalarAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int *p = &(int){ 7 };
        int main() {
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, fileScopeCompoundLiteralStructAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; int y; };
        struct S *p = &(struct S){ 3, 4 };
        int main() {
            printf("%d %d", p->x, p->y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

TEST(Compiler, fileScopeCompoundLiteralMemberAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; int y; };
        int *p = &((struct S){ 3, 4 }).y;
        int main() {
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

} // namespace
