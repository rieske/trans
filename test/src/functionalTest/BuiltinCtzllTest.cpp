#include "TestFixtures.h"

namespace {

TEST(Compiler, builtinCtzllOfEight) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d", __builtin_ctzll(8));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, builtinCtzllOfOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d", __builtin_ctzll(1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, builtinCtzllHighBit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned long x;
            x = 0x8000000000000000UL;
            printf("%d", __builtin_ctzll(x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("63");
}

TEST(Compiler, builtinCtzllEwahWrapperShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        #define ewah_bit_ctz64(x) __builtin_ctzll(x)
        int main(void) {
            unsigned long word;
            int offset;
            word = 12;
            offset = 2;
            printf("%d", offset + ewah_bit_ctz64(word >> offset));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, builtinCtzOfEight) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d", __builtin_ctz(8));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, builtinCtzllWrongArityIsError) {
    SourceProgram program{R"prg(
        int main(void) {
            return __builtin_ctzll();
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("wrong number of arguments to __builtin_ctzll");
}

TEST(Compiler, isoStdRejectsBuiltinCtzll) {
    SourceProgram program{R"prg(
        int main(void) {
            return __builtin_ctzll(8);
        }
    )prg", std::vector<std::string> { "-std=c" }};
    program.compile();
    program.assertCompilationErrors("symbol `__builtin_ctzll` is not defined");
}

} // namespace
