#include "TestFixtures.h"

namespace {

TEST(Compiler, simpleAdditionAndSubtraction) {
    SourceProgram program{R"prg(
        int main() {
            int first, second;
            scanf("%d %d", &first, &second);
            printf("%d ", first+second);
            printf("%d ", second+first);
            printf("%d ", first-second);
            printf("%d", second-first);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0 0 0");
    program.runAndExpect("0 1", "1 1 -1 1");
    program.runAndExpect("1 0", "1 1 1 -1");
    program.runAndExpect("1 1", "2 2 0 0");
}

TEST(Compiler, simpleMultiplication) {
    SourceProgram program{R"prg(
        int main() {
            int first, second;
            int firstProduct, secondProduct;
            scanf("%d %d", &first, &second);
            firstProduct = first*second;
            secondProduct = second*first;
            printf("%d %d ", firstProduct, secondProduct);
            printf("%d ", firstProduct == secondProduct);
            printf("%d", firstProduct != secondProduct);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0 1 0");
    program.runAndExpect("0 1", "0 0 1 0");
    program.runAndExpect("1 0", "0 0 1 0");
    program.runAndExpect("-1 0", "0 0 1 0");
    program.runAndExpect("1 1", "1 1 1 0");
    program.runAndExpect("-1 1", "-1 -1 1 0");
    program.runAndExpect("1 -1", "-1 -1 1 0");
    program.runAndExpect("-1 -1", "1 1 1 0");
    program.runAndExpect("1 2", "2 2 1 0");
    program.runAndExpect("2 1", "2 2 1 0");
    program.runAndExpect("2 2", "4 4 1 0");
}

// FIXME: %ld - ints treated as longs for now
TEST(Compiler, simpleDivision) {
    SourceProgram program{R"prg(
        int main() {
            int first, second;
            scanf("%ld %ld", &first, &second);
            printf("%d", first/second);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 1", "1");
    program.runAndExpect("2 1", "2");
    program.runAndExpect("2 2", "1");
    program.runAndExpect("4 2", "2");
    program.runAndExpect("15 3", "5");

    program.runAndExpect("2 3", "0");
    program.runAndExpect("3 2", "1");
    program.runAndExpect("5 2", "2");
}

// FIXME: %ld - ints treated as longs for now
TEST(Compiler, simpleModulus) {
    SourceProgram program{R"prg(
        int main() {
            int first, second;
            scanf("%ld %ld", &first, &second);
            printf("%d", first%second);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 1", "0");
    program.runAndExpect("2 1", "0");
    program.runAndExpect("4 2", "0");
    program.runAndExpect("15 3", "0");

    program.runAndExpect("2 3", "2");
    program.runAndExpect("3 2", "1");
    program.runAndExpect("5 2", "1");
}

}
