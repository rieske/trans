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

}

