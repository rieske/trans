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

// FIXME
/*
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
*/
}

