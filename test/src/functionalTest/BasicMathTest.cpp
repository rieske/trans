#include "TestFixtures.h"

namespace {

TEST(Compiler, simpleAdditionAndSubtraction) {
    SourceProgram program{R"prg(
        int main() {
            int first;
            int second;
            input first;
            input second;
            output first+second;
            output second+first;
            output first-second;
            output second-first;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0\n", "0\n0\n0\n0\n");
    program.runAndExpect("0\n1\n", "1\n1\n-1\n1\n");
    program.runAndExpect("1\n0\n", "1\n1\n1\n-1\n");
    program.runAndExpect("1\n1\n", "2\n2\n0\n0\n");
}

TEST(Compiler, simpleMultiplication) {
    SourceProgram program{R"prg(
        int main() {
            int first;
            int second;
            int firstProduct;
            int secondProduct;
            input first;
            input second;
            firstProduct = first*second;
            secondProduct = second*first;
            output firstProduct;
            output secondProduct;
            output firstProduct == secondProduct;
            output firstProduct != secondProduct;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0\n", "0\n0\n1\n0\n");
    program.runAndExpect("0\n1\n", "0\n0\n1\n0\n");
    program.runAndExpect("1\n0\n", "0\n0\n1\n0\n");
    program.runAndExpect("-1\n0\n", "0\n0\n1\n0\n");
    program.runAndExpect("1\n1\n", "1\n1\n1\n0\n");
    program.runAndExpect("-1\n1\n", "-1\n-1\n1\n0\n");
    program.runAndExpect("1\n-1\n", "-1\n-1\n1\n0\n");
    program.runAndExpect("-1\n-1\n", "1\n1\n1\n0\n");
    program.runAndExpect("1\n2\n", "2\n2\n1\n0\n");
    program.runAndExpect("2\n1\n", "2\n2\n1\n0\n");
    program.runAndExpect("2\n2\n", "4\n4\n1\n0\n");
}

TEST(Compiler, simpleDivision) {
    SourceProgram program{R"prg(
        int main() {
            int first;
            int second;
            input first;
            input second;
            output first/second;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n1\n", "0\n");
    program.runAndExpect("1\n1\n", "1\n");
    program.runAndExpect("2\n1\n", "2\n");
    program.runAndExpect("2\n2\n", "1\n");
    program.runAndExpect("4\n2\n", "2\n");
    program.runAndExpect("15\n3\n", "5\n");

    program.runAndExpect("2\n3\n", "0\n");
    program.runAndExpect("3\n2\n", "1\n");
    program.runAndExpect("5\n2\n", "2\n");
}

TEST(Compiler, simpleModulus) {
    SourceProgram program{R"prg(
        int main() {
            int first;
            int second;
            input first;
            input second;
            output first%second;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n1\n", "0\n");
    program.runAndExpect("1\n1\n", "0\n");
    program.runAndExpect("2\n1\n", "0\n");
    program.runAndExpect("4\n2\n", "0\n");
    program.runAndExpect("15\n3\n", "0\n");

    program.runAndExpect("2\n3\n", "2\n");
    program.runAndExpect("3\n2\n", "1\n");
    program.runAndExpect("5\n2\n", "1\n");
}

}
