#include "TestFixtures.h"

namespace {

TEST(Compiler, fileScopeDecayedArrayPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        char *p = slop;
        int main(void) {
            slop[0] = 88;
            printf("%d %d", p == slop, p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 88");
}

TEST(Compiler, fileScopeGitStrbufInitShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        struct strbuf g = { .buf = slop };
        int main(void) {
            slop[0] = 65;
            printf("%d %d %d %d", g.alloc == 0, g.len == 0, g.buf == slop, g.buf[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1 65");
}

TEST(Compiler, fileScopeStrbufPositionalInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        struct strbuf g = { 0, 0, slop };
        int main(void) {
            printf("%d", g.buf == slop);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, automaticDecayedArrayPointerInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        int main(void) {
            char *p = slop;
            slop[0] = 88;
            printf("%d %d", p == slop, p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 88");
}

TEST(Compiler, automaticGitStrbufInitShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        int main(void) {
            struct strbuf s = { .buf = slop };
            slop[0] = 65;
            printf("%d %d %d %d", s.alloc == 0, s.len == 0, s.buf == slop, s.buf[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1 65");
}

TEST(Compiler, automaticStrbufPositionalInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        int main(void) {
            struct strbuf s = { 0, 0, slop };
            slop[0] = 66;
            printf("%d %d", s.buf == slop, s.buf[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 66");
}

TEST(Compiler, functionScopeStaticStrbufInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        char *buf_of(void) {
            static struct strbuf sb = { .buf = slop };
            return sb.buf;
        }
        int main(void) {
            slop[0] = 66;
            printf("%d %d", buf_of() == slop, buf_of()[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 66");
}

TEST(Compiler, functionScopeStaticStrbufPool) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        int check(void) {
            static struct strbuf pool[2] = { { .buf = slop }, { .buf = slop } };
            return pool[0].buf == slop && pool[1].buf == slop;
        }
        int main(void) {
            printf("%d", check());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, functionScopeStaticDecayedLocalArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *buf_of(void) {
            static char slop[1];
            static char *p = slop;
            slop[0] = 67;
            return p;
        }
        int main(void) {
            char *p;
            p = buf_of();
            printf("%d %d", p[0], buf_of() == p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("67 1");
}

TEST(Compiler, fileScopeAddressOfObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = 7;
        int *p = &g;
        int main(void) {
            printf("%d %d", p == &g, *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 7");
}

TEST(Compiler, functionScopeStaticAddressOfFileScope) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = 9;
        int read(void) {
            static int *p = &g;
            return *p;
        }
        int main(void) {
            printf("%d", read());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, decayedArrayEqualsAddressOfArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        char *a = slop;
        char (*b)[1] = &slop;
        int main(void) {
            printf("%d", a == (char *)b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeStringPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *p = "hi";
        int main(void) {
            printf("%s", p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("hi");
}

TEST(Compiler, fileScopeFunctionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int foo(void) {
            return 3;
        }
        int (*fp)(void) = foo;
        int main(void) {
            printf("%d", fp());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, fileScopeAddressOfFunction) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int foo(void) {
            return 4;
        }
        int (*fp)(void) = &foo;
        int main(void) {
            printf("%d", fp());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, fileScopeNullPointerStillFolds) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *p = 0;
        struct S {
            char *buf;
        };
        struct S g = { .buf = 0 };
        int main(void) {
            printf("%d %d", p == 0, g.buf == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, fileScopeExternArrayPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern char slop[1];
        char *p = slop;
        char slop[1];
        int main(void) {
            slop[0] = 70;
            printf("%d %d", p == slop, p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 70");
}

TEST(Compiler, staticInitRejectsAutomaticAddress) {
    SourceProgram program{R"prg(
        int main(void) {
            int x;
            static int *p = &x;
            return p == 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("global initializer is not a constant expression");
}

TEST(Compiler, fileScopeFunctionInitToIntIsError) {
    SourceProgram program{R"prg(
        int foo(void) {
            return 1;
        }
        int g = foo;
        int main(void) {
            return g;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("function designator used as a value is not supported");
}

TEST(Compiler, fileScopeFunctionBraceInitToIntIsError) {
    SourceProgram program{R"prg(
        int foo(void) {
            return 1;
        }
        int g = { foo };
        int main(void) {
            return g;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("function designator used as a value is not supported");
}

TEST(Compiler, fileScopeArrayInitToFunctionPointerIsError) {
    SourceProgram program{R"prg(
        char slop[1];
        int (*fp)(void) = slop;
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

TEST(Compiler, fileScopeUnionAddressThenByteLastWins) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        union U {
            char *p;
            char c;
        };
        union U g = { .p = slop, .c = 1 };
        int main(void) {
            printf("%d", g.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeFloatFromInteger) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        float f = 2;
        int main(void) {
            printf("%d", (int)(f + f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, fileScopeDoubleFromInteger) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        double d = 2;
        int main(void) {
            printf("%d", (int)(d + d));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, fileScopeIntFromFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = 2.5f;
        int main(void) {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, fileScopeIntFromNegativeFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = -2.5f;
        int main(void) {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-2");
}

TEST(Compiler, fileScopeBoolFromFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        bool b = 2.5f;
        int main(void) {
            printf("%d", b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeBoolFromSmallFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        bool b = 0.1f;
        int main(void) {
            printf("%d", b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeBoolFromZeroFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        bool b = 0.0f;
        int main(void) {
            printf("%d", b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, fileScopeFloatFromDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        float f = 2.5;
        int main(void) {
            printf("%d", (int)(f + f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, fileScopeStructFloatThenIntPack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            float f;
            int i;
        };
        struct S g = { 2.5f, 3 };
        int main(void) {
            printf("%d %d", (int)(g.f + g.f), g.i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 3");
}

TEST(Compiler, fileScopeStructIntThenFloatPack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int i;
            float f;
        };
        struct S g = { 3, 2.5f };
        int main(void) {
            printf("%d %d", g.i, (int)(g.f + g.f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 5");
}

} // namespace
