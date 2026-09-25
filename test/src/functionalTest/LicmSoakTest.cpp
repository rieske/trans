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

TEST(Compiler, licmInvariantLoad) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int soak(int *p, int n) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += *p;
            }
            return s;
        }
        int main(void) {
            int v;
            v = 4;
            printf("%d", soak(&v, 5));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, licmLoadSeesStoreInLoop) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int soak(int *p, int *q, int n) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += *p;
                *q = *q + 1;
            }
            return s;
        }
        int main(void) {
            int v;
            v = 1;
            printf("%d", soak(&v, &v, 3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, licmLoadSeesCallInLoop) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void bump(int *p) { *p = *p + 1; }
        int soak(int *p, int n) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += *p;
                bump(p);
            }
            return s;
        }
        int main(void) {
            int v;
            v = 1;
            printf("%d", soak(&v, 3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, licmLoadSeesDirectStore) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int soak(void) {
            int x;
            int *p;
            int i;
            int s;
            x = 1;
            p = &x;
            s = 0;
            for (i = 0; i < 3; i++) {
                s += *p;
                x = x + 1;
            }
            return s;
        }
        int main(void) {
            printf("%d", soak());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, licmLoadSeesIncrement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int soak(void) {
            int x;
            int *p;
            int i;
            int s;
            x = 1;
            p = &x;
            s = 0;
            for (i = 0; i < 3; i++) {
                s += *p;
                x++;
            }
            return s;
        }
        int main(void) {
            printf("%d", soak());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, licmHeaderLoadSeesDirectStore) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int soak(void) {
            int x;
            int *p;
            int n;
            int s;
            x = 1;
            p = &x;
            n = 3;
            s = 0;
            do {
                s += *p;
                x = x + 1;
                n = n - 1;
            } while (n);
            return s;
        }
        int main(void) {
            printf("%d", soak());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, licmHeaderLoadSeesIncrement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int soak(void) {
            int x;
            int *p;
            int n;
            int s;
            x = 1;
            p = &x;
            n = 3;
            s = 0;
            do {
                s += *p;
                x++;
                n = n - 1;
            } while (n);
            return s;
        }
        int main(void) {
            printf("%d", soak());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, licmHeaderLoadSeesCall) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void bump(int *p) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            *p = *p + 1;
            (void)buf[0];
        }
        int soak(void) {
            int x;
            int *p;
            int n;
            int s;
            x = 1;
            p = &x;
            n = 3;
            s = 0;
            do {
                s += *p;
                bump(p);
                n = n - 1;
            } while (n);
            return s;
        }
        int main(void) {
            printf("%d", soak());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, licmLoadNotRunOnZeroTrip) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int soak(int *p, int n) {
            int i;
            int s;
            s = 0;
            for (i = 0; i < n; i++) {
                s += *p;
            }
            return s;
        }
        int main(void) {
            int *p;
            p = 0;
            printf("%d", soak(p, 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

} // namespace
