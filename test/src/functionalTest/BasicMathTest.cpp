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

// Signed idiv requires cqo (sign-extend rax into rdx:rax). Using xor rdx,rdx
// treats a negative dividend as a huge unsigned value and yields garbage (or
// SIGFPE). git parse-options OPT_INTEGER uses -2345 / 10 for negative args.
TEST(Compiler, signedDivisionOfNegative) {
    SourceProgram program{R"prg(
        int main() {
            int first, second;
            scanf("%ld %ld", &first, &second);
            printf("%d", first/second);
            return 0;
        }
    )prg"};

    program.compile();

    // Toward zero (C99+): -5/2 = -2, 5/-2 = -2, -5/-2 = 2
    program.runAndExpect("-1 1", "-1");
    program.runAndExpect("1 -1", "-1");
    program.runAndExpect("-1 -1", "1");
    program.runAndExpect("-5 2", "-2");
    program.runAndExpect("5 -2", "-2");
    program.runAndExpect("-5 -2", "2");
    program.runAndExpect("-20 6", "-3");
    program.runAndExpect("-2345 10", "-234");
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

// Same idiv setup bug for %: remainder must match (a/b)*b + (a%b) == a.
TEST(Compiler, signedModulusOfNegative) {
    SourceProgram program{R"prg(
        int main() {
            int first, second;
            scanf("%ld %ld", &first, &second);
            printf("%d", first%second);
            return 0;
        }
    )prg"};

    program.compile();

    // Toward-zero div: -20%6 = -2, 20%-6 = 2, -20%-6 = -2
    program.runAndExpect("-1 1", "0");
    program.runAndExpect("-5 2", "-1");
    program.runAndExpect("5 -2", "1");
    program.runAndExpect("-5 -2", "-1");
    program.runAndExpect("-20 6", "-2");
    program.runAndExpect("20 -6", "2");
    program.runAndExpect("-20 -6", "-2");
    program.runAndExpect("-2345 10", "-5");
}

// Constants (not scanf) exercise the same codegen path used by git helpers.
TEST(Compiler, signedDivisionAndModuloConstants) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            a = -2345;
            b = 10;
            printf("%d %d", a / b, a % b);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("-234 -5");
}

// Unsigned division of values with high bit set must use `div`, not signed `idiv`.
// idiv of SIZE_MAX (rax=0xff..ff, rdx=0) overflows the signed quotient and raises SIGFPE
// (git st_mult / unsigned_mult_overflows: max / a).
TEST(Compiler, unsignedDivisionOfMaxValue) {
    SourceProgram program{R"prg(
        int main() {
            unsigned long maxv;
            unsigned long a;
            unsigned long q;
            maxv = 0xffffffffffffffffUL;
            a = 1;
            q = maxv / a;
            printf("%lu", q);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("18446744073709551615");
}

// git unsigned_mult_overflows / st_mult: (a && b > max / a).
TEST(Compiler, unsignedMultOverflowCheck) {
    SourceProgram program{R"prg(
        int st_mult(unsigned long a, unsigned long b) {
            unsigned long maxv;
            maxv = 0xffffffffffffffffUL;
            if (a) {
                if (b > maxv / a) {
                    return -1;
                }
            }
            return (int)(a * b);
        }
        int main() {
            printf("%d %d %d", st_mult(1, 65), st_mult(8, 8), st_mult(0, 99));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65 64 0");
}

// Unsigned modulo of high-bit values also needs unsigned div.
TEST(Compiler, unsignedModuloOfMaxValue) {
    SourceProgram program{R"prg(
        int main() {
            unsigned long maxv;
            unsigned long a;
            maxv = 0xffffffffffffffffUL;
            a = 3;
            printf("%lu", maxv % a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

}
