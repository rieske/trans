#include "TestFixtures.h"

namespace {

TEST(Compiler, equals) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a == b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "1\n");
    program.runAndExpect("1\n1", "1\n");
    program.runAndExpect("42\n42", "1\n");
    program.runAndExpect("-1\n-1", "1\n");
    program.runAndExpect("-42\n-42", "1\n");

    program.runAndExpect("0\n1", "0\n");
    program.runAndExpect("1\n0", "0\n");
    program.runAndExpect("0\n-1", "0\n");
    program.runAndExpect("-1\n0", "0\n");
    program.runAndExpect("42\n-42", "0\n");
    program.runAndExpect("-42\n42", "0\n");
}

TEST(Compiler, notEquals) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a != b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0\n");
    program.runAndExpect("1\n1", "0\n");
    program.runAndExpect("42\n42", "0\n");
    program.runAndExpect("-1\n-1", "0\n");
    program.runAndExpect("-42\n-42", "0\n");

    program.runAndExpect("0\n1", "1\n");
    program.runAndExpect("1\n0", "1\n");
    program.runAndExpect("0\n-1", "1\n");
    program.runAndExpect("-1\n0", "1\n");
    program.runAndExpect("42\n-42", "1\n");
    program.runAndExpect("-42\n42", "1\n");
}

// FIXME: fails to compile - something gets messed up with labels in generated assembly
/*TEST(Compiler, equalsNegated) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output !(a == b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0\n");
    program.runAndExpect("1\n1", "0\n");
    program.runAndExpect("42\n42", "0\n");
    program.runAndExpect("-1\n-1", "0\n");
    program.runAndExpect("-42\n-42", "0\n");

    program.runAndExpect("0\n1", "1\n");
    program.runAndExpect("1\n0", "1\n");
    program.runAndExpect("0\n-1", "1\n");
    program.runAndExpect("-1\n0", "1\n");
    program.runAndExpect("42\n-42", "1\n");
    program.runAndExpect("-42\n42", "1\n");
}*/

TEST(Compiler, lessThanOrEqualsConst) {
    SourceProgram program{R"prg(
        int main() {
            output 0 <= 0;
            output 1 <= 1;
            output 42 <= 42;
            output -1 <= -1;
            output -42 <= -42;

            output 0 <= 1;
            output 1 <= 0;
            output 0 <= -1;
            output -1 <= 0;
            output 42 <= -42;
            output -42 <= 42;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1\n1\n1\n1\n1\n1\n0\n0\n1\n0\n1\n");
}

TEST(Compiler, lessThanOrEquals) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a <= b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "1\n");
    program.runAndExpect("1\n1", "1\n");
    program.runAndExpect("42\n42", "1\n");
    program.runAndExpect("-1\n-1", "1\n");
    program.runAndExpect("-42\n-42", "1\n");

    program.runAndExpect("0\n1", "1\n");
    program.runAndExpect("1\n0", "0\n");
    // FIXME:
    //program.runAndExpect("0\n-1", "0\n");
    //program.runAndExpect("-1\n0", "1\n");
    //program.runAndExpect("42\n-42", "0\n");
    //program.runAndExpect("-42\n42", "1\n");
}

TEST(Compiler, lessThanConst) {
    SourceProgram program{R"prg(
        int main() {
            output 0 < 0;
            output 1 < 1;
            output 42 < 42;
            output -1 < -1;
            output -42 < -42;

            output 0 < 1;
            output 1 < 0;
            output 0 < -1;
            output -1 < 0;
            output 42 < -42;
            output -42 < 42;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0\n0\n0\n0\n1\n0\n0\n1\n0\n1\n");
}

TEST(Compiler, lessThan) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a < b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0\n");
    program.runAndExpect("1\n1", "0\n");
    program.runAndExpect("42\n42", "0\n");
    program.runAndExpect("-1\n-1", "0\n");
    program.runAndExpect("-42\n-42", "0\n");

    program.runAndExpect("0\n1", "1\n");
    program.runAndExpect("1\n0", "0\n");
    // FIXME:
    //program.runAndExpect("0\n-1", "0\n");
    //program.runAndExpect("-1\n0", "1\n");
    //program.runAndExpect("42\n-42", "0\n");
    //program.runAndExpect("-42\n42", "1\n");
}

TEST(Compiler, moreThanConst) {
    SourceProgram program{R"prg(
        int main() {
            output 0 > 0;
            output 1 > 1;
            output 42 > 42;
            output -1 > -1;
            output -42 > -42;

            output 0 > 1;
            output 1 > 0;
            output 0 > -1;
            output -1 > 0;
            output 42 > -42;
            output -42 > 42;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0\n0\n0\n0\n0\n1\n1\n0\n1\n0\n");
}

TEST(Compiler, moreThan) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a > b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0\n");
    program.runAndExpect("1\n1", "0\n");
    program.runAndExpect("42\n42", "0\n");
    program.runAndExpect("-1\n-1", "0\n");
    program.runAndExpect("-42\n-42", "0\n");

    program.runAndExpect("0\n1", "0\n");
    program.runAndExpect("1\n0", "1\n");
    // FIXME:
    //program.runAndExpect("0\n-1", "1\n");
    //program.runAndExpect("-1\n0", "0\n");
    //program.runAndExpect("42\n-42", "1\n");
    //program.runAndExpect("-42\n42", "0\n");
}

TEST(Compiler, moreThanOrEqualsConst) {
    SourceProgram program{R"prg(
        int main() {
            output 0 >= 0;
            output 1 >= 1;
            output 42 >= 42;
            output -1 >= -1;
            output -42 >= -42;

            output 0 >= 1;
            output 1 >= 0;
            output 0 >= -1;
            output -1 >= 0;
            output 42 >= -42;
            output -42 >= 42;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1\n1\n1\n1\n1\n0\n1\n1\n0\n1\n0\n");
}

TEST(Compiler, moreThanOrEquals) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a >= b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "1\n");
    program.runAndExpect("1\n1", "1\n");
    program.runAndExpect("42\n42", "1\n");
    program.runAndExpect("-1\n-1", "1\n");
    program.runAndExpect("-42\n-42", "1\n");

    program.runAndExpect("0\n1", "0\n");
    program.runAndExpect("1\n0", "1\n");
    // FIXME:
    //program.runAndExpect("0\n-1", "1\n");
    //program.runAndExpect("-1\n0", "0\n");
    //program.runAndExpect("42\n-42", "1\n");
    //program.runAndExpect("-42\n42", "0\n");
}

TEST(Compiler, ifEquality) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            if (a == b) {
                output 1;
            } else {
                output 0;
            }
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "1\n");
    program.runAndExpect("1\n1", "1\n");
    program.runAndExpect("42\n42", "1\n");
    program.runAndExpect("-1\n-1", "1\n");
    program.runAndExpect("-42\n-42", "1\n");

    program.runAndExpect("0\n1", "0\n");
    program.runAndExpect("1\n0", "0\n");
    program.runAndExpect("0\n-1", "0\n");
    program.runAndExpect("-1\n0", "0\n");
    program.runAndExpect("42\n-42", "0\n");
    program.runAndExpect("-42\n42", "0\n");
    program.runAndExpect("10\n11", "0\n");
}

TEST(Compiler, simpleIfConditional) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            input i;
            if (i == 0) {
                output 0;
            }
            if (i < 0) {
                output -1;
            }
            if (i > 0) {
                output 1;
            }
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0\n");
    program.runAndExpect("1", "1\n");

    // FIXME:
    //program.runAndExpect("-1", "-1\n");
}

}
