#include "TestFixtures.h"

namespace {

TEST(Compiler, increments) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            scanf("%d", &n);
            printf("%d ", n++);
            printf("%d ", n);
            printf("%d ", ++n);
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "1 2 3 3");
    program.runAndExpect("0", "0 1 2 2");
    program.runAndExpect("-1", "-1 0 1 1");
    program.runAndExpect("-3", "-3 -2 -1 -1");
}

TEST(Compiler, decrements) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            scanf("%d", &n);
            printf("%d ", n--);
            printf("%d ", n);
            printf("%d ", --n);
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("3", "3 2 1 1");
    program.runAndExpect("2", "2 1 0 0");
    program.runAndExpect("1", "1 0 -1 -1");
    program.runAndExpect("-1", "-1 -2 -3 -3");
}

TEST(Compiler, incrementsFunctions) {
    SourceProgram program{R"prg(
        int pre(int i) {
            return ++i;
        }

        int post(int i) {
            return i++;
        }

        int main() {
            int n;
            scanf("%d", &n);
            printf("%d ", pre(n));
            printf("%d ", post(n));
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "2 1 1");
    program.runAndExpect("0", "1 0 0");
    program.runAndExpect("-1", "0 -1 -1");
    program.runAndExpect("-3", "-2 -3 -3");
}

TEST(Compiler, decrementsFunctions) {
    SourceProgram program{R"prg(
        int pre(int i) {
            return --i;
        }

        int post(int i) {
            return i--;
        }

        int main() {
            int n;
            scanf("%d", &n);
            printf("%d ", pre(n));
            printf("%d ", post(n));
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("3", "2 3 3");
    program.runAndExpect("2", "1 2 2");
    program.runAndExpect("1", "0 1 1");
    program.runAndExpect("-1", "-2 -1 -1");
}

TEST(Compiler, incrementsFunctionsPointers) {
    SourceProgram program{R"prg(
        void pre(int* i) {
            ++(*i);
            return;
        }

        void post(int* i) {
            (*i)++;
            return;
        }

        int main() {
            int n;
            scanf("%d", &n);
            pre(&n);
            printf("%d\n", n);
            post(&n);
            printf("%d\n", n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "2\n3\n");
    program.runAndExpect("0", "1\n2\n");
    program.runAndExpect("-1", "0\n1\n");
    program.runAndExpect("-3", "-2\n-1\n");
}

TEST(Compiler, prefixIncrementThroughPointer) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 5;
            p = &n;
            ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, postfixIncrementThroughPointer) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 5;
            p = &n;
            (*p)++;
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, prefixDecrementThroughPointer) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 5;
            p = &n;
            --(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, postfixDecrementThroughPointer) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 5;
            p = &n;
            (*p)--;
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, postfixIncrementThroughPointerExpressionValue) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            int old;
            n = 5;
            p = &n;
            old = (*p)++;
            printf("%d %d", old, n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("5 6");
}

TEST(Compiler, prefixIncrementThroughPointerExpressionValue) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            int neu;
            n = 5;
            p = &n;
            neu = ++(*p);
            printf("%d %d", neu, n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("6 6");
}

// Pointer variable ++/-- advances by sizeof(pointee), not 1 byte.
// git main: argv++; handle_options: (*argv)++.
TEST(Compiler, pointerPostfixIncrementScales) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = a;
            p++;
            printf("%d", *p);
            p++;
            printf(" %d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

TEST(Compiler, pointerPrefixIncrementScales) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = a;
            ++p;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, pointerPostfixDecrementScales) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = a;
            p = p + 2;
            p--;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

// char** ++ advances one pointer slot (8 bytes). git: argv++; (*argv)++.
TEST(Compiler, pointerToPointerPostfixIncrement) {
    SourceProgram program{R"prg(
        int handle(const char ***argv, int *argc) {
            while (*argc > 0) {
                const char *cmd;
                cmd = (*argv)[0];
                if (cmd[0] != '-')
                    break;
                if (cmd[1] == 'v') {
                    printf("v");
                    return 1;
                }
                (*argv)++;
                (*argc)--;
            }
            return 0;
        }
        int main() {
            const char *args[3];
            const char **p;
            int n;
            args[0] = "-x";
            args[1] = "hello";
            args[2] = 0;
            p = args;
            n = 2;
            handle(&p, &n);
            printf("%s %d", p[0], n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("hello 1");
}

// char* ++ advances one byte.
TEST(Compiler, charPointerPostfixIncrement) {
    SourceProgram program{R"prg(
        int main() {
            char s[4];
            char *p;
            s[0] = 97;
            s[1] = 98;
            s[2] = 99;
            s[3] = 0;
            p = s;
            p++;
            printf("%c", *p);
            p++;
            printf("%c", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("bc");
}

}

