#include "TestFixtures.h"

namespace {

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
