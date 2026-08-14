#include "TestFixtures.h"

namespace {

TEST(Compiler, fileScopeCharArrayTableFromStrings) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static char colors[][8] = {
            "ab",
            "cd",
        };
        int main(void) {
            printf("%d %d %d %d %d %d", colors[0][0], colors[0][1], colors[0][2],
                colors[1][0], colors[1][1], colors[1][2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("97 98 0 99 100 0");
}

TEST(Compiler, fileScopeGitColorTableEscapes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static char colors[][8] = {
            "\033[m",
            "\033[33m",
        };
        int main(void) {
            printf("%d %d %d %d %d %d %d", colors[0][0], colors[0][1], colors[0][2],
                colors[1][0], colors[1][1], colors[1][2], colors[1][3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("27 91 109 27 91 51 51");
}

TEST(Compiler, fileScopeStructCharArrayFromString) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            char name[8];
            int n;
        };
        struct S s = { "hi", 3 };
        int main(void) {
            printf("%d %d %d %d", s.name[0], s.name[1], s.name[2], s.n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3");
}

TEST(Compiler, fileScopeBracedStringInCharArrayRow) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static char rows[][4] = {
            { "ab" },
            { "c" },
        };
        int main(void) {
            printf("%d %d %d %d %d", rows[0][0], rows[0][1], rows[0][2],
                rows[1][0], rows[1][1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("97 98 0 99 0");
}

TEST(Compiler, fileScopeCharArrayRowPadsWithZeros) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static char row[][5] = { "hi" };
        int main(void) {
            printf("%d %d %d %d %d", row[0][0], row[0][1], row[0][2],
                row[0][3], row[0][4]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 0 0");
}

TEST(Compiler, fileScopeCharArrayRowTruncatesNul) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static char row[][2] = { "hi" };
        int main(void) {
            printf("%d %d", row[0][0], row[0][1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105");
}

TEST(Compiler, fileScopeCharArrayRowExcessIsError) {
    SourceProgram program{R"prg(
        static char row[][1] = { "hi" };
        int main(void) {
            return row[0][0] == 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements in array initializer");
}

TEST(Compiler, functionScopeStaticCharArrayTable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *hint(void) {
            static char colors[][8] = { "ab", "cd" };
            return colors[1];
        }
        int main(void) {
            printf("%s", hint());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("cd");
}

TEST(Compiler, automaticCharArrayTableFromStrings) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            char colors[][8] = { "ab", "cd" };
            printf("%s %s", colors[0], colors[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ab cd");
}

TEST(Compiler, automaticBracedStringInCharArrayRow) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            char rows[][4] = { { "ab" }, { "c" } };
            printf("%d %d %d %d %d", rows[0][0], rows[0][1], rows[0][2],
                rows[1][0], rows[1][1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("97 98 0 99 0");
}

TEST(Compiler, fileScopeDesignatedCharArrayFromString) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int n;
            char name[8];
        };
        struct S s = { .name = "hi", .n = 3 };
        int main(void) {
            printf("%d %d %d %d", s.name[0], s.name[1], s.name[2], s.n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3");
}

TEST(Compiler, fileScopeNestedCurrentObjectCharArrayFromString) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Inner {
            char a[4];
            int x;
        };
        struct Outer {
            struct Inner inner;
            int y;
        };
        struct Outer o = { "hi", 3 };
        int main(void) {
            printf("%d %d %d %d %d", o.inner.a[0], o.inner.a[1], o.inner.a[2],
                o.inner.x, o.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 0 3 0");
}

} // namespace
