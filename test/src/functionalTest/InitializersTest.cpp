#include "TestFixtures.h"

namespace {

TEST(Compiler, scalarBraceInitializer) {
    SourceProgram program{R"prg(
        int main() {
            int x = { 7 };
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, scalarNonBraceInitializerStillWorks) {
    SourceProgram program{R"prg(
        int main() {
            int x = 11;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}

TEST(Compiler, structBraceInitializer) {
    SourceProgram program{R"prg(
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S s = { 1, 2 };
            printf("%d %d", s.x, s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, structPartialBraceZeroFills) {
    SourceProgram program{R"prg(
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S s = { 5 };
            printf("%d %d", s.x, s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 0");
}

TEST(Compiler, structBraceTrailingComma) {
    SourceProgram program{R"prg(
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S s = { 3, 4, };
            printf("%d %d", s.x, s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

TEST(Compiler, structBraceExcessElementsIsError) {
    SourceProgram program{R"prg(
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S s = { 1, 2, 3 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements");
}

TEST(Compiler, scalarBraceExcessElementsIsError) {
    SourceProgram program{R"prg(
        int main() {
            int x = { 1, 2 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements");
}

// Nested braces for aggregate members are supported (Phase 1.5 NestedInitTest).
TEST(Compiler, nestedBraceInitializerAccepted) {
    SourceProgram program{R"prg(
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };

        int main() {
            struct Outer o = { { 1, 2 }, 3 };
            printf("%d %d %d", o.in.a, o.in.b, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, flatInitOfStructMemberIsError) {
    SourceProgram program{R"prg(
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };

        int main() {
            struct Outer o = { 1, 2 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("aggregate member");
}

} // namespace
