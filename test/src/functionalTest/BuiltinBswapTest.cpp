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

TEST(Compiler, builtinBswapRoundTrip) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned short s;
            unsigned u;
            unsigned long l;
            s = 0x1234;
            u = 0x12345678;
            l = 1;
            printf("%d %u %d",
                (int)__builtin_bswap16(__builtin_bswap16(s)),
                __builtin_bswap32(__builtin_bswap32(u)),
                (int)(__builtin_bswap64(__builtin_bswap64(l)) == l));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4660 305419896 1");
}

TEST(Compiler, builtinBswap16TruncatesWiderArgument) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned x;
            x = 0x00ab1234;
            printf("%d", (int)__builtin_bswap16(x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("13330");
}

TEST(Compiler, builtinBswap32OfSum) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned x;
            unsigned y;
            x = 0x12000000;
            y = 0x00345678;
            printf("%u", __builtin_bswap32(x + y));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2018915346");
}

TEST(Compiler, builtinBswapMatchesGlibcWrapperShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static __inline unsigned short __bswap_16(unsigned short x) {
            return __builtin_bswap16(x);
        }
        static __inline unsigned __bswap_32(unsigned x) {
            return __builtin_bswap32(x);
        }
        __extension__ static __inline unsigned long __bswap_64(unsigned long x) {
            return __builtin_bswap64(x);
        }
        int main() {
            unsigned long z;
            z = 1;
            printf("%d %u %d", (int)__bswap_16(0x1234), __bswap_32(0x12345678),
                (int)(__bswap_64(z) >> 56));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("13330 2018915346 1");
}

TEST(Compiler, builtinBswapWrongArityIsError) {
    SourceProgram program{R"prg(
        int main() {
            return (int)__builtin_bswap32();
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("wrong number of arguments to __builtin_bswap32");
}

TEST(Compiler, isoStdRejectsBuiltinBswap) {
    SourceProgram program{R"prg(
        int main() {
            return (int)__builtin_bswap32(1);
        }
    )prg"};
    program.addCompilerArg("-std=c");
    program.compile();
    program.assertCompilationErrors("symbol `__builtin_bswap32` is not defined");
}

} // namespace
