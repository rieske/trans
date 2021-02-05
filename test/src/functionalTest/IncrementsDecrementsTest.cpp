#include "TestFixtures.h"

namespace {

TEST(Compiler, increments) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            input n;
            output n++;
            output n;
            output ++n;
            output n;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "1\n2\n3\n3\n");
    program.runAndExpect("0", "0\n1\n2\n2\n");
    program.runAndExpect("-1", "-1\n0\n1\n1\n");
    program.runAndExpect("-3", "-3\n-2\n-1\n-1\n");
}

TEST(Compiler, decrements) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            input n;
            output n--;
            output n;
            output --n;
            output n;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("3", "3\n2\n1\n1\n");
    program.runAndExpect("2", "2\n1\n0\n0\n");
    program.runAndExpect("1", "1\n0\n-1\n-1\n");
    program.runAndExpect("-1", "-1\n-2\n-3\n-3\n");
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
            input n;
            output pre(n);
            output post(n);
            output n;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "2\n1\n1\n");
    program.runAndExpect("0", "1\n0\n0\n");
    program.runAndExpect("-1", "0\n-1\n-1\n");
    program.runAndExpect("-3", "-2\n-3\n-3\n");
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
            input n;
            output pre(n);
            output post(n);
            output n;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("3", "2\n3\n3\n");
    program.runAndExpect("2", "1\n2\n2\n");
    program.runAndExpect("1", "0\n1\n1\n");
    program.runAndExpect("-1", "-2\n-1\n-1\n");
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
            input n;
            pre(&n);
            output n;
            post(&n);
            output n;
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

