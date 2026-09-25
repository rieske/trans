#include "TestFixtures.h"

namespace {

TEST(Compiler, arrayElementStoreReachesPointer) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int main(void) {
            int a[2];
            int *p;
            p = a;
            *p = 7;
            a[0] = 3;
            printf("%d", *p);
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, callThroughElementAddressUpdatesReload) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        void g(int *p) { *p = 9; }
        int main(void) {
            int a[2];
            int *p;
            p = a;
            *p = 7;
            g(&a[0]);
            printf("%d", *p);
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, callThroughFieldAddressUpdatesReload) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        void g(int *p) { *p = 9; }
        struct S { int a; };
        int main(void) {
            struct S s;
            int *p;
            p = (int *)&s;
            *p = 7;
            g(&s.a);
            printf("%d", *p);
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, intStoreThroughFloatIsOne) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int main(void) {
            float x;
            int *p;
            p = (int *)&x;
            *p = 0x3f800000;
            printf("%d", x == 1.0f);
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, earlierPointerTargetSurvivesRetarget) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        void foo(int *p) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            *p = 99;
            (void)buf[0];
        }
        int main(void) {
            int x;
            int y;
            int *p;
            x = 1;
            y = 2;
            p = &x;
            foo(p);
            p = &y;
            printf("%d", x + *p);
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("101");
}

TEST(Compiler, escapingPointerEscapesItsTarget) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        void foo(int **pp) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            **pp = 99;
            (void)buf[0];
        }
        int h(void) {
            int x;
            int *p;
            x = 1;
            p = &x;
            foo(&p);
            return x;
        }
        int main(void) {
            printf("%d", h());
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("99");
}

TEST(Compiler, returnAfterEscapingPointer) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        void foo(int **pp) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            **pp = 99;
            (void)buf[0];
        }
        int h(void) {
            int x;
            int *p;
            x = 1;
            p = &x;
            foo(&p);
            return x;
        }
        int main(void) {
            printf("%d", h());
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("99");
}

TEST(Compiler, loadedPointerArgumentEscapesTarget) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        void foo(int *p) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            *p = 99;
            (void)buf[0];
        }
        int h(void) {
            int x;
            int *p;
            int **pp;
            x = 1;
            p = &x;
            pp = &p;
            foo(*pp);
            return x;
        }
        int main(void) {
            printf("%d", h());
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("99");
}

TEST(Compiler, earlierMayTargetSurvivesJoin) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        void foo(int *p) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            *p = 99;
            (void)buf[0];
        }
        int h(int c) {
            int n;
            int x;
            int y;
            int *p;
            n = 1;
            char buf[n];
            buf[0] = 0;
            y = 2;
            p = &x;
            if (c) {
                p = &y;
            }
            x = 3;
            foo(p);
            (void)buf[0];
            return x;
        }
        int main(void) {
            printf("%d", h(0));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("99");
}

TEST(Compiler, loopCopyMissesLaterTarget) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int *g;
        void stash(int *p) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            g = p;
            (void)buf[0];
        }
        void poke(void) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            *g = 99;
            (void)buf[0];
        }
        int h(int n) {
            int x;
            int y;
            int *p;
            int *q;
            int i;
            x = 10;
            y = 20;
            p = &x;
            q = &x;
            for (i = 0; i < n; i++) {
                q = p;
                p = &y;
            }
            stash(q);
            y = 5;
            poke();
            return y;
        }
        int main(void) {
            printf("%d", h(2));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("99");
}

TEST(Compiler, laterStoreDoesNotSurviveStashedPointer) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int *g;
        void stash(int *p) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            g = p;
            (void)buf[0];
        }
        void poke(void) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            *g = 99;
            (void)buf[0];
        }
        int h(void) {
            int n;
            int x;
            int *p;
            int **pp;
            n = 1;
            char buf[n];
            buf[0] = 0;
            x = 1;
            p = &x;
            pp = &p;
            stash(*pp);
            x = 2;
            poke();
            (void)buf[0];
            return x;
        }
        int main(void) {
            printf("%d", h());
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("99");
}

TEST(Compiler, copiedValueSurvivesCallOnSource) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        void poke(int *q) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            *q = 7;
            (void)buf[0];
        }
        int h(void) {
            int y;
            int x;
            int *p;
            y = 1;
            p = &x;
            x = y;
            poke(&y);
            return *p;
        }
        int main(void) {
            printf("%d", h());
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, globalPointerEscapesLocal) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int *g;
        void poke(void) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            *g = 7;
            (void)buf[0];
        }
        int h(void) {
            int y;
            y = 1;
            g = &y;
            poke();
            return y;
        }
        int main(void) {
            printf("%d", h());
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, loadedPointerAcrossJoinEscapesTarget) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        void foo(int *q) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            *q = 99;
            (void)buf[0];
        }
        int h(int c) {
            int x;
            int y;
            int *p;
            int **pp;
            int *q;
            x = 1;
            y = 2;
            p = &x;
            pp = &p;
            if (c) {
                q = *pp;
            } else {
                q = &y;
            }
            x = 3;
            foo(q);
            return x;
        }
        int main(int argc, char **argv) {
            (void)argv;
            printf("%d", h(argc));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("99");
}

TEST(Compiler, charStoreKeepsOtherBytes) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int h(void) {
            int x;
            char *p;
            x = 0x01020304;
            p = (char *)&x;
            *p = 1;
            return x;
        }
        int main(void) {
            printf("%d", h());
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("16909057");
}

} // namespace
