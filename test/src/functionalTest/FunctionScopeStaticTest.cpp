#include "TestFixtures.h"

namespace {

TEST(Compiler, functionScopeStaticIntPersists) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int next(void) {
            static int n;
            n = n + 1;
            return n;
        }
        int main(void) {
            printf("%d %d %d", next(), next(), next());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, functionScopeStaticWithInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int bump(void) {
            static int n = 10;
            n = n + 1;
            return n;
        }
        int main(void) {
            printf("%d %d", bump(), bump());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 12");
}

TEST(Compiler, functionScopeStaticBufferReturn) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *getbuf(void) {
            static char buf[8];
            static int n;
            n = n + 1;
            buf[0] = 65;
            buf[1] = 48 + n;
            buf[2] = 0;
            return buf;
        }
        int main(void) {
            char *p;
            p = getbuf();
            printf("%s ", p);
            p = getbuf();
            printf("%s", p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("A1 A2");
}

TEST(Compiler, functionScopeStaticsArePerFunction) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int a(void) {
            static int n;
            n = n + 1;
            return n;
        }
        int b(void) {
            static int n;
            n = n + 10;
            return n;
        }
        int main(void) {
            printf("%d %d %d %d", a(), a(), b(), b());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 10 20");
}

TEST(Compiler, functionScopeStaticIncompleteArrayTakesInitSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int size_and_sum(void) {
            static int a[] = {1, 2, 3};
            return (int)(sizeof(a) / sizeof(a[0])) + a[0] + a[1] + a[2];
        }
        int main(void) {
            printf("%d", size_and_sum());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, functionScopeStaticArrayInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int walk(int i) {
            static int a[2] = {10, 20};
            a[i] = a[i] + 1;
            return a[i];
        }
        int main(void) {
            printf("%d %d %d %d", walk(0), walk(0), walk(1), walk(1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 12 21 22");
}

TEST(Compiler, functionScopeStaticStructInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        int walk(int which) {
            static struct S s = {10, 20};
            if (which) {
                s.a = s.a + 1;
                return s.a;
            }
            s.b = s.b + 1;
            return s.b;
        }
        int main(void) {
            printf("%d %d %d %d", walk(1), walk(1), walk(0), walk(0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 12 21 22");
}

// Static-local init must write the function-scope home, not a same-named global.
TEST(Compiler, functionScopeStaticShadowingGlobalKeepsBothInits) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = 1;
        int getGlobal(void) {
            return g;
        }
        int main(void) {
            static int g = 2;
            printf("%d %d", g, getGlobal());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

TEST(Compiler, functionScopeStaticsArePerBlock) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int walk(int which) {
            if (which) {
                static int n;
                n = n + 1;
                return n;
            } else {
                static int n;
                n = n + 10;
                return n;
            }
        }
        int main(void) {
            printf("%d %d %d %d", walk(1), walk(1), walk(0), walk(0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 10 20");
}

} // namespace
