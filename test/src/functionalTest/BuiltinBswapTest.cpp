#include "TestFixtures.h"

namespace {

TEST(Compiler, builtinBswap16) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned short x;
            x = 0x1234;
            printf("%d", (int)__builtin_bswap16(x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("13330");
}

TEST(Compiler, builtinBswap32) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned x;
            x = 0x12345678;
            printf("%u", __builtin_bswap32(x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2018915346");
}

TEST(Compiler, builtinBswap64) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long x;
            unsigned long y;
            x = 1;
            y = __builtin_bswap64(x);
            printf("%d", (int)(y >> 56));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, builtinBswapMatchesGlibcWrapperShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static __inline unsigned short __bswap_16(unsigned short x) {
            return __builtin_bswap16(x);
        }
        static __inline unsigned __bswap_32(unsigned x) {
            return __builtin_bswap32(x);
        }
        int main() {
            printf("%d %u", (int)__bswap_16(0x1234), __bswap_32(0x12345678));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("13330 2018915346");
}

TEST(Compiler, builtinBswapWrongArityIsError) {
    SourceProgram program{R"prg(
        int main() {
            return (int)__builtin_bswap32();
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("no match for function");
}

TEST(Compiler, isoStdRejectsBuiltinBswap) {
    SourceProgram program{R"prg(
        int main() {
            return (int)__builtin_bswap32(1);
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("symbol `__builtin_bswap32` is not defined");
}

} // namespace
