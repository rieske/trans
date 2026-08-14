#include "TestFixtures.h"

namespace {

TEST(Compiler, globalStructBraceInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        struct S g = { 1, 2 };
        int main() {
            printf("%d %d", g.a, g.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, globalStructPartialZeroFill) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int a;
            int b;
            int c;
        };
        struct S g = { 5 };
        int main() {
            printf("%d %d %d", g.a, g.b, g.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 0 0");
}

TEST(Compiler, globalStructDesignatedInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int a;
            int b;
            int c;
        };
        struct S g = { .b = 42 };
        int main() {
            printf("%d %d %d", g.a, g.b, g.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 42 0");
}

TEST(Compiler, globalNestedDesignatedInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct Flags {
            int recursive;
            int dense;
        };
        struct Prune {
            void *orderfile;
            int reverse;
            struct Flags flags;
        };
        struct Rev {
            int remerge_diff;
            struct Prune pruning;
            int limited;
        };
        struct Rev g = {
            .pruning.flags.recursive = 1
        };
        int main() {
            if (g.pruning.orderfile != 0) return 1;
            if (g.pruning.reverse != 0) return 2;
            if (g.pruning.flags.recursive != 1) return 3;
            if (g.pruning.flags.dense != 0) return 4;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, globalArrayBraceInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int a[3] = { 1, 2, 3 };
        int main() {
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, globalFlatNestedStruct) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        struct Outer g = { 1, 2, 3 };
        int main() {
            printf("%d %d %d", g.in.a, g.in.b, g.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, fileScopeFloatLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static float g = 2.5f;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, fileScopeDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static double g = 2.5;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, fileScopeNegativeFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static float g = -2.5f;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-2");
}

// Fuzz: file-scope long double constants were emitted as zero (16-byte x87 init).
TEST(Compiler, fileScopeLongDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long double g = 3.0L;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, fileScopeLongDoubleNonInteger) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long double g = 8.5L;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, fileScopeLongDoubleNegative) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long double g = -3.0L;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-3");
}

TEST(Compiler, fileScopeStructLongDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { long double x; };
        static struct S g = { 3.0L };
        int main(void) {
            printf("%d", (int)g.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, fileScopeDoubleFromLongDouble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static double d = 1.0L;
        int main(void) {
            printf("%d", (int)d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeIntFromLongDouble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int i = 1.0L;
        int main(void) {
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeIntFromNegativeLongDouble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int i = -3.0L;
        int main(void) {
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-3");
}

TEST(Compiler, functionScopeStaticLongDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        long double get(void) {
            static long double g = 5.0L;
            return g;
        }
        int main(void) {
            printf("%d", (int)get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

} // namespace
