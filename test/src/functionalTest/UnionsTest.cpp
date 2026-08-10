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

} // namespace

