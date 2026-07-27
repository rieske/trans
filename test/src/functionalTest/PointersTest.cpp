#include "TestFixtures.h"

#include <fstream>

namespace {

TEST(Compiler, writesThroughPointerArgumentBeforeAnyCall) {
    SourceProgram program{R"prg(
        int f(int* v) {
            *v = *v + 1;
            printf("%d\n", *v);
            return 0;
        }

        int main() {
            int a = 5;
            f(&a);
            printf("%d\n", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6\n6\n");
}

TEST(Compiler, compilesSwapProgram) {
    SourceProgram program{R"prg(
        int swap(int *x, int *y) {
            int temp;
            printf("%d\n%d\n", *x, *y);
            temp = *x;
            *x = *y;
            *y = temp;
            printf("%d\n%d\n", *x, *y);
            return 0;
        }

        int main() {
            int a = 0;
            int b = 1;
            printf("%d\n%d\n", a, b);
            printf("%d\n%d\n", &a, &b);
            swap (&a, &b);
            printf("%d\n%d\n", a, b);
            printf("%d\n%d\n", &a, &b);
            return 0;
        }
    )prg"};
    program.compile();
    program.run();

    std::ifstream expectedOutputStream{program.getOutputFilePath()};
    std::string outputLine;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    std::string firstAddressBefore;
    expectedOutputStream >> firstAddressBefore;
    std::string secondAddressBefore;
    expectedOutputStream >> secondAddressBefore;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    std::string firstAddressAfter;
    expectedOutputStream >> firstAddressAfter;
    std::string secondAddressAfter;
    expectedOutputStream >> secondAddressAfter;
    EXPECT_THAT(firstAddressBefore, Not(Eq(secondAddressBefore)));
    EXPECT_THAT(firstAddressBefore, Eq(firstAddressAfter));
    EXPECT_THAT(secondAddressBefore, Eq(secondAddressAfter));
}

// TODO(gap): multi-level pointers (`int**`, `**pp`) - declarator/type only models a
// single pointer level reliably; `**pp` is type-checked as unary `*` on `int`
// ("invalid type argument of unary *"). Need recursive pointer types in the type
// system and declarator lowering, plus codegen for multi-level load/store.
/*
TEST(Compiler, pointerToPointer) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int* p;
            int** pp;
            a = 5;
            p = &a;
            pp = &p;
            printf("%d %d", **pp, *p);
            **pp = 9;
            printf(" %d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 5 9");
}
*/

// Pointer arithmetic must scale by pointee size (int stride 4). Found by targeted probing
// during mutfuzz campaign: p+1 / p-q used raw byte math.

TEST(Compiler, pointerPlusIntScalesByElementSize) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[0];
            printf("%d ", *(p + 1));
            printf("%d", *(p + 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

TEST(Compiler, pointerMinusPointerIsElementCount) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            int *q;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = &a[2];
            q = &a[0];
            printf("%d", p - q);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, pointerIncrementScalesByElementSize) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[0];
            p++;
            printf("%d ", *p);
            ++p;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

// int + ptr is commutative with ptr + int (IntPlusPtr IR form).
TEST(Compiler, intPlusPointerScalesByElementSize) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[0];
            printf("%d ", *(1 + p));
            printf("%d", *(2 + p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

// ptr - int scales the index (PtrMinusInt IR form).
TEST(Compiler, pointerMinusIntScalesByElementSize) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[2];
            printf("%d ", *(p - 1));
            printf("%d", *(p - 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 10");
}

// Pointer -- / prefix -- step by pointee size (non-unit Dec quad).
TEST(Compiler, pointerDecrementScalesByElementSize) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[2];
            p--;
            printf("%d ", *p);
            --p;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 10");
}

TEST(Compiler, charPointerDifferenceIsBytes) {
    SourceProgram program{R"prg(
        int main() {
            char a[3];
            char *p;
            char *q;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = &a[2];
            q = &a[0];
            printf("%d", p - q);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

} // namespace
