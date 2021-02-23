#include "TestFixtures.h"

namespace {

TEST(Compiler, simpleAdditionAndSubtraction) {
    SourceProgram program{R"prg(
        int main() {
            int first;
            int second;
            input first;
            input second;
            printf("%d ", first+second);
            printf("%d ", second+first);
            printf("%d ", first-second);
            printf("%d", second-first);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0 0 0 0");
    program.runAndExpect("0\n1", "1 1 -1 1");
    program.runAndExpect("1\n0", "1 1 1 -1");
    program.runAndExpect("1\n1", "2 2 0 0");
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
            printf("%d %d ", firstProduct, secondProduct);
            printf("%d ", firstProduct == secondProduct);
            printf("%d", firstProduct != secondProduct);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0 0 1 0");
    program.runAndExpect("0\n1", "0 0 1 0");
    program.runAndExpect("1\n0", "0 0 1 0");
    program.runAndExpect("-1\n0", "0 0 1 0");
    program.runAndExpect("1\n1", "1 1 1 0");
    program.runAndExpect("-1\n1", "-1 -1 1 0");
    program.runAndExpect("1\n-1", "-1 -1 1 0");
    program.runAndExpect("-1\n-1", "1 1 1 0");
    program.runAndExpect("1\n2", "2 2 1 0");
    program.runAndExpect("2\n1", "2 2 1 0");
    program.runAndExpect("2\n2", "4 4 1 0");
}

TEST(Compiler, simpleDivision) {
    SourceProgram program{R"prg(
        int main() {
            int first;
            int second;
            input first;
            input second;
            printf("%d", first/second);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n1", "0");
    program.runAndExpect("1\n1", "1");
    program.runAndExpect("2\n1", "2");
    program.runAndExpect("2\n2", "1");
    program.runAndExpect("4\n2", "2");
    program.runAndExpect("15\n3", "5");

    program.runAndExpect("2\n3", "0");
    program.runAndExpect("3\n2", "1");
    program.runAndExpect("5\n2", "2");
}

TEST(Compiler, simpleModulus) {
    SourceProgram program{R"prg(
        int main() {
            int first;
            int second;
            input first;
            input second;
            printf("%d", first%second);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n1", "0");
    program.runAndExpect("1\n1", "0");
    program.runAndExpect("2\n1", "0");
    program.runAndExpect("4\n2", "0");
    program.runAndExpect("15\n3", "0");

    program.runAndExpect("2\n3", "2");
    program.runAndExpect("3\n2", "1");
    program.runAndExpect("5\n2", "1");
}

}
