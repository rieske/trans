#include "TestFixtures.h"

namespace {

TEST(Compiler, equals) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a == b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "1");
    program.runAndExpect("1\n1", "1");
    program.runAndExpect("42\n42", "1");
    program.runAndExpect("-1\n-1", "1");
    program.runAndExpect("-42\n-42", "1");

    program.runAndExpect("0\n1", "0");
    program.runAndExpect("1\n0", "0");
    program.runAndExpect("0\n-1", "0");
    program.runAndExpect("-1\n0", "0");
    program.runAndExpect("42\n-42", "0");
    program.runAndExpect("-42\n42", "0");
}

TEST(Compiler, notEquals) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a != b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0");
    program.runAndExpect("1\n1", "0");
    program.runAndExpect("42\n42", "0");
    program.runAndExpect("-1\n-1", "0");
    program.runAndExpect("-42\n-42", "0");

    program.runAndExpect("0\n1", "1");
    program.runAndExpect("1\n0", "1");
    program.runAndExpect("0\n-1", "1");
    program.runAndExpect("-1\n0", "1");
    program.runAndExpect("42\n-42", "1");
    program.runAndExpect("-42\n42", "1");
}

// FIXME: fails to compile - something gets messed up with labels in generated assembly
/*TEST(Compiler, equalsNegated) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", !(a == b));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0");
    program.runAndExpect("1\n1", "0");
    program.runAndExpect("42\n42", "0");
    program.runAndExpect("-1\n-1", "0");
    program.runAndExpect("-42\n-42", "0");

    program.runAndExpect("0\n1", "1");
    program.runAndExpect("1\n0", "1");
    program.runAndExpect("0\n-1", "1");
    program.runAndExpect("-1\n0", "1");
    program.runAndExpect("42\n-42", "1");
    program.runAndExpect("-42\n42", "1");
}*/

TEST(Compiler, lessThanOrEqualsConst) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d ", 0 <= 0);
            printf("%d ", 1 <= 1);
            printf("%d ", 42 <= 42);
            printf("%d ", -1 <= -1);
            printf("%d ", -42 <= -42);

            printf("%d ", 0 <= 1);
            printf("%d ", 1 <= 0);
            printf("%d ", 0 <= -1);
            printf("%d ", -1 <= 0);
            printf("%d ", 42 <= -42);
            printf("%d", -42 <= 42);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 1 1 1 1 1 0 0 1 0 1");
}

TEST(Compiler, lessThanOrEquals) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a <= b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "1");
    program.runAndExpect("1\n1", "1");
    program.runAndExpect("42\n42", "1");
    program.runAndExpect("-1\n-1", "1");
    program.runAndExpect("-42\n-42", "1");

    program.runAndExpect("0\n1", "1");
    program.runAndExpect("1\n0", "0");
    program.runAndExpect("0\n-1", "0");
    program.runAndExpect("-1\n0", "1");
    program.runAndExpect("42\n-42", "0");
    program.runAndExpect("-42\n42", "1");
}

TEST(Compiler, lessThanConst) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d ", 0 < 0);
            printf("%d ", 1 < 1);
            printf("%d ", 42 < 42);
            printf("%d ", -1 < -1);
            printf("%d ", -42 < -42);

            printf("%d ", 0 < 1);
            printf("%d ", 1 < 0);
            printf("%d ", 0 < -1);
            printf("%d ", -1 < 0);
            printf("%d ", 42 < -42);
            printf("%d", -42 < 42);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0 0 0 0 1 0 0 1 0 1");
}

TEST(Compiler, lessThan) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a < b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0");
    program.runAndExpect("1\n1", "0");
    program.runAndExpect("42\n42", "0");
    program.runAndExpect("-1\n-1", "0");
    program.runAndExpect("-42\n-42", "0");

    program.runAndExpect("0\n1", "1");
    program.runAndExpect("1\n0", "0");
    program.runAndExpect("0\n-1", "0");
    program.runAndExpect("-1\n0", "1");
    program.runAndExpect("42\n-42", "0");
    program.runAndExpect("-42\n42", "1");
}

TEST(Compiler, moreThanConst) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d ", 0 > 0);
            printf("%d ", 1 > 1);
            printf("%d ", 42 > 42);
            printf("%d ", -1 > -1);
            printf("%d ", -42 > -42);

            printf("%d ", 0 > 1);
            printf("%d ", 1 > 0);
            printf("%d ", 0 > -1);
            printf("%d ", -1 > 0);
            printf("%d ", 42 > -42);
            printf("%d", -42 > 42);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0 0 0 0 0 1 1 0 1 0");
}

TEST(Compiler, moreThan) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a > b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0");
    program.runAndExpect("1\n1", "0");
    program.runAndExpect("42\n42", "0");
    program.runAndExpect("-1\n-1", "0");
    program.runAndExpect("-42\n-42", "0");

    program.runAndExpect("0\n1", "0");
    program.runAndExpect("1\n0", "1");
    program.runAndExpect("0\n-1", "1");
    program.runAndExpect("-1\n0", "0");
    program.runAndExpect("42\n-42", "1");
    program.runAndExpect("-42\n42", "0");
}

TEST(Compiler, moreThanOrEqualsConst) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d ", 0 >= 0);
            printf("%d ", 1 >= 1);
            printf("%d ", 42 >= 42);
            printf("%d ", -1 >= -1);
            printf("%d ", -42 >= -42);

            printf("%d ", 0 >= 1);
            printf("%d ", 1 >= 0);
            printf("%d ", 0 >= -1);
            printf("%d ", -1 >= 0);
            printf("%d ", 42 >= -42);
            printf("%d", -42 >= 42);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 1 1 1 1 0 1 1 0 1 0");
}

TEST(Compiler, moreThanOrEquals) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a >= b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "1");
    program.runAndExpect("1\n1", "1");
    program.runAndExpect("42\n42", "1");
    program.runAndExpect("-1\n-1", "1");
    program.runAndExpect("-42\n-42", "1");

    program.runAndExpect("0\n1", "0");
    program.runAndExpect("1\n0", "1");
    program.runAndExpect("0\n-1", "1");
    program.runAndExpect("-1\n0", "0");
    program.runAndExpect("42\n-42", "1");
    program.runAndExpect("-42\n42", "0");
}

TEST(Compiler, ifEquality) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            if (a == b) {
                printf("%d", 1);
            } else {
                printf("%d", 0);
            }
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "1");
    program.runAndExpect("1\n1", "1");
    program.runAndExpect("42\n42", "1");
    program.runAndExpect("-1\n-1", "1");
    program.runAndExpect("-42\n-42", "1");

    program.runAndExpect("0\n1", "0");
    program.runAndExpect("1\n0", "0");
    program.runAndExpect("0\n-1", "0");
    program.runAndExpect("-1\n0", "0");
    program.runAndExpect("42\n-42", "0");
    program.runAndExpect("-42\n42", "0");
    program.runAndExpect("10\n11", "0");
}

TEST(Compiler, simpleIfConditional) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            input i;
            if (i == 0) {
                printf("%d", 0);
            }
            if (i < 0) {
                printf("%d", -1);
            }
            if (i > 0) {
                printf("%d", 1);
            }
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0");
    program.runAndExpect("1", "1");

    program.runAndExpect("-1", "-1");
}

}
