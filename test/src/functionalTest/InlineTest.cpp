#include "TestFixtures.h"

namespace {

TEST(Compiler, inlinedStaticLocalPersists) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int next(void) {
            static int n;
            n = n + 1;
            return n;
        }
        int main(void) {
            printf("%d %d %d", next(), next(), next());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, functionPointerToStaticHelperStillCalls) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int add1(int x) {
            return x + 1;
        }
        int main(void) {
            int (*fp)(int) = add1;
            printf("%d %d", add1(41), fp(40));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42 41");
}

TEST(Compiler, recursiveFactorialKeepsCorrectResult) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int fact(int n) {
            if (n <= 1) {
                return 1;
            }
            return n * fact(n - 1);
        }
        int main(void) {
            printf("%d %d", fact(5), fact(0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("120 1");
}

TEST(Compiler, variadicPrintfStillFormatsAfterInline) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int add1(int x) {
            return x + 1;
        }
        int main(void) {
            printf("%d %s", add1(41), "ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42 ok");
}

} // namespace
