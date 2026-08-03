#include "TestFixtures.h"

#include <string>
#include <vector>

// Object address must satisfy addr % align(T) == 0. String literals are a
// packed byte stream in .data, so a following object is the alignment case.

namespace {

TEST(Compiler, staticLongAfterStringIsEightAligned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long x;
        int main(void) {
            printf("%d", (int)((unsigned long)&x % 8));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, staticMutexAfterStringIsEightAligned) {
    SourceProgram program{R"prg(
        #include <pthread.h>
        int printf(const char *, ...);
        static pthread_mutex_t mu;
        int main(void) {
            printf("%d", (int)((unsigned long)&mu % 8));
            return 0;
        }
    )prg",
            std::vector<std::string>{"-pthread"}};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, staticInt128AfterStringIsSixteenAligned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static __int128 x;
        int main(void) {
            printf("%d", (int)((unsigned long)&x % 16));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, staticLongAfterCharIsEightAligned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static char c;
        static long x;
        int main(void) {
            c = 1;
            printf("%d", (int)((unsigned long)&x % 8));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, functionScopeStaticLongAfterStringIsEightAligned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            static long x;
            printf("%d", (int)((unsigned long)&x % 8));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, automaticInt128FirstLocalIsSixteenAligned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            __int128 x;
            x = 2;
            printf("%d", (int)((unsigned long)&x % 16));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, automaticInt128AfterTwoCharsIsSixteenAligned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            char a;
            char b;
            __int128 x;
            a = 1;
            b = 2;
            x = 3;
            printf("%d", (int)((unsigned long)&x % 16));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, automaticLongDoubleFirstLocalIsSixteenAligned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            long double x;
            x = 2.0L;
            printf("%d", (int)((unsigned long)&x % 16));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

} // namespace
