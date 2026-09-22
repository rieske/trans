#include "TestFixtures.h"

namespace {

TEST(Compiler, licmForInvariantAdd) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += n + 1;
            }
            return s;
        }

        int main() {
            printf("%d", soak(4));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, licmForThatDoesNotRun) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n) {
            int i;
            int s;
            s = 7;
            for (i = 0; i < n; i++) {
                s += n + 1;
            }
            return s;
        }

        int main() {
            printf("%d", soak(0));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, licmNestedInvariantAdd) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n) {
            int i;
            int j;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                for (j = 0; j < n; j++) {
                    s += n + 1;
                }
            }
            return s;
        }

        int main() {
            printf("%d", soak(3));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("36");
}

TEST(Compiler, licmNamedAddAliasedByStore) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int soak(int n) {
            int i;
            int s;
            int *p;
            s = 0;
            p = &n;
            for (i = 0; i < 3; i++) {
                *p = *p + 1;
                s = s + n;
            }
            return s;
        }

        int main() {
            printf("%d", soak(1));
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("9");
}

} // namespace
