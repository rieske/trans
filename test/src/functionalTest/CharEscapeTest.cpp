#include "TestFixtures.h"

namespace {

TEST(Compiler, simpleCharEscapesHaveCValues) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d %d %d %d %d %d %d %d %d %d %d",
                '\a', '\b', '\f', '\v', '\?', '\\', '\'', '"', '\n', '\t', '\r');
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8 12 11 63 92 39 34 10 9 13");
}

TEST(Compiler, hexCharEscapeIsUnsignedByte) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d %d %d %d", '\x41', '\xFE', '\xff', (int)sizeof('\xFE'));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65 254 255 4");
}

TEST(Compiler, octalCharEscapeIsByte) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d %d %d", '\0', '\033', '\101');
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 27 65");
}

TEST(Compiler, staticHexCharInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static const char bom[] = {'\xFE', '\xFF', '\0', '\xFE'};
        int main(void) {
            printf("%d %d %d %d",
                (unsigned char)bom[0], (unsigned char)bom[1],
                (unsigned char)bom[2], (unsigned char)bom[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("254 255 0 254");
}

TEST(Compiler, switchOnFormFeedVerticalTabAndOctal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int classify(int c) {
            switch (c) {
            case '\f':
                return 1;
            case '\v':
                return 2;
            case '\033':
                return 3;
            default:
                return 0;
            }
        }
        int main(void) {
            printf("%d %d %d %d", classify('\f'), classify('\v'),
                classify('\033'), classify('x'));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 0");
}

TEST(Compiler, enumAndCaseUseHexCharIce) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { FE = '\xFE' };
        int main(void) {
            int c = FE;
            switch (c) {
            case '\xFE':
                printf("%d", FE);
                break;
            default:
                printf("0");
                break;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("254");
}

} // namespace
