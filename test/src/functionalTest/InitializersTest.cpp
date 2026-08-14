#include "TestFixtures.h"

namespace {

TEST(Compiler, scalarBraceInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

// Nested brace runtime pin: NestedInitTest.nestedStructBraceInitializer.
// Flat current-object init: FlatCurrentObjectInitTest.

TEST(Compiler, braceZeroPointerStructClearsDirtyBits) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct hashfd_options {
            char *progress;
            unsigned long buffer_len;
        };
        void poison(void) {
            volatile unsigned long junk[64];
            int i;
            for (i = 0; i < 64; i = i + 1) {
                junk[i] = 0x5555444433332222UL;
            }
        }
        int check(void) {
            struct hashfd_options opts = { 0 };
            return (opts.progress == 0) + (opts.buffer_len == 0);
        }
        int main() {
            poison();
            printf("%d", check());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

} // namespace
