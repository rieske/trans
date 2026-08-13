#include "TestFixtures.h"

namespace {

TEST(Compiler, unionBasicOverlay) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        union U {
            int i;
            int j;
        };

        int main() {
            union U u;
            u.i = 42;
            printf("%d", u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, unionSizeIsMaxMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        union U {
            int i;
            int *p;
        };

        int main() {
            union U u;
            int x;
            x = 7;
            u.p = &x;
            printf("%d %d", *u.p, (int)sizeof(union U));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8");
}

TEST(Compiler, unionPointerArrow) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        union U {
            int i;
            int j;
        };

        int main() {
            union U u;
            union U *pu;
            pu = &u;
            pu->i = 9;
            printf("%d", u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, namedUnionInsideStruct) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int tag;
            union U {
                int i;
                int j;
            } u;
        };

        int main() {
            struct S s;
            s.tag = 1;
            s.u.i = 42;
            printf("%d %d %d", s.tag, s.u.i, s.u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 42 42");
}



TEST(Compiler, unionBraceInitializesFirstMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        union U {
            int i;
            int j;
        };
        int main() {
            union U u = { 7 };
            printf("%d %d", u.i, u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 7");
}

TEST(Compiler, unionZeroBraceFirstMember) {
    // Grammar requires a non-empty initializer_list; zero first arm via { 0 }.
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        union U {
            int i;
            int j;
        };
        int main() {
            union U u = { 0 };
            printf("%d %d", u.i, u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, constIntUnionMemberIsComplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        union U {
            const int n;
            int m;
        };
        int main() {
            union U u;
            u.m = 4;
            printf("%d %d", u.m, (int)sizeof(union U));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4");
}

// GNU transparent union: pointer members accept matching pointers and null (socket APIs).
TEST(Compiler, transparentUnionAcceptsMemberPointers) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct sa { int fam; };
        struct sun { int fam; char path[8]; };
        typedef union {
            struct sa *__sa;
            struct sun *__sun;
        } sockarg_t __attribute__((__transparent_union__));
        static int take(sockarg_t a) {
            return a.__sa != 0;
        }
        int main(void) {
            struct sa s;
            struct sun u;
            s.fam = 1;
            u.fam = 2;
            printf("%d %d %d", take(&s), take(&u), take(0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 0");
}

// transparent_union on a prior non-typedef declaration must not mark a later union.
TEST(Compiler, transparentUnionAttributeDoesNotLeakAcrossDeclarations) {
    SourceProgram program{R"prg(
        struct sa { int fam; };
        int not_a_union __attribute__((__transparent_union__));
        typedef union {
            struct sa *__sa;
        } plain_u;
        static int take(plain_u a) {
            (void)a;
            return 0;
        }
        int main(void) {
            struct sa s;
            return take(&s);
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

} // namespace

