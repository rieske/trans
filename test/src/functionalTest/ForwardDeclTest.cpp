#include "TestFixtures.h"

namespace {

TEST(Compiler, forwardDeclarationThenDefinition) {
    SourceProgram program{R"prg(
        int foo(int x);

        int main() {
            printf("%d", foo(3));
            return 0;
        }

        int foo(int x) {
            return x + 1;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, mutualRecursion) {
    SourceProgram program{R"prg(
        int isOdd(int n);

        int isEven(int n) {
            if (n) {
                return isOdd(n - 1);
            } else {
                return 1;
            }
        }

        int isOdd(int n) {
            if (n) {
                return isEven(n - 1);
            } else {
                return 0;
            }
        }

        int main() {
            printf("%d %d %d %d", isEven(0), isEven(1), isOdd(0), isOdd(3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 0 1");
}

TEST(Compiler, prototypeOnlyThenCallAfterDefinition) {
    SourceProgram program{R"prg(
        int bar(int a, int b);

        int bar(int a, int b) {
            return a * b;
        }

        int main() {
            printf("%d", bar(6, 7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Multi-spec `unsigned` / `unsigned int` must be the same type (git: mix of both).
TEST(Compiler, unsignedIntPrototypeMatchesUnsigned) {
    SourceProgram program{R"prg(
        unsigned f(void);
        unsigned int f(void) {
            return 7;
        }
        unsigned int g(unsigned int x);
        unsigned g(unsigned x) {
            return x + 1;
        }
        int main() {
            printf("%d %d", f(), g(41));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 42");
}

// C ignores top-level const on parameters for prototype compatibility
// (git: const char * vs const char * const).
TEST(Compiler, topLevelConstParamCompatible) {
    SourceProgram program{R"prg(
        int f(const char *s);
        int f(const char * const s) {
            return *s;
        }
        int g(char *p);
        int g(char * const p) {
            return *p;
        }
        int main() {
            char c;
            c = 65;
            printf("%d %d", f(&c), g(&c));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65 65");
}

} // namespace
