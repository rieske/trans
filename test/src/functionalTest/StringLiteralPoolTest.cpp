#include "TestFixtures.h"

namespace {

TEST(Compiler, stringLiteralWithApostrophe) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%s", "account's");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("account's");
}

TEST(Compiler, stringLiteralGitIdentQuoteShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("got '%s'", "x");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("got 'x'");
}

TEST(Compiler, stringLiteralWithDoubleQuotes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%s", "say \"hi\"");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("say \"hi\"");
}

TEST(Compiler, fileScopeStringLiteralWithApostrophe) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        const char *p = "it's";
        int main(void) {
            printf("%s", p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("it's");
}

TEST(Compiler, stringLiteralApostropheAndNewline) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%s", "it's\nok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("it's\nok");
}

} // namespace
