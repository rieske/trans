#include "TestFixtures.h"

namespace {

const char* kWordHelpers = R"prg(
        void set_words(__int128 *x, unsigned long lo, unsigned long hi) {
            unsigned long *p;
            p = (unsigned long *)x;
            p[0] = lo;
            p[1] = hi;
        }
        void print_words(__int128 x) {
            unsigned long *p;
            p = (unsigned long *)&x;
            printf("%d %d", (int)p[0], (int)p[1]);
        }
)prg";

TEST(Compiler, int128AddNoCarry) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 1, 0);
            set_words(&b, 2, 0);
            print_words(a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 0");
}

TEST(Compiler, int128AddCarryIntoHighWord) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 one;
            a = -1;
            set_words(&one, 1, 0);
            print_words(a + one);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, int128AddHighWords) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 42, 1);
            set_words(&b, 1, 1);
            print_words(a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("43 2");
}

TEST(Compiler, int128AddTwoNegatives) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            a = -1;
            print_words(a + a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-2 -1");
}

TEST(Compiler, int128AddMixedLong) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            long b;
            set_words(&a, 0, 1);
            b = 1;
            print_words(a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, int128SubBorrowFromHighWord) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 one;
            set_words(&a, 0, 0);
            set_words(&one, 1, 0);
            print_words(a - one);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, int128SubHighWord) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 one;
            set_words(&a, 0, 1);
            set_words(&one, 1, 0);
            print_words(a - one);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 0");
}

TEST(Compiler, int128BitwiseBothWords) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 3, 12);
            set_words(&b, 5, 10);
            print_words(a & b);
            printf(" ");
            print_words(a | b);
            printf(" ");
            print_words(a ^ b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 8 7 14 6 6");
}

TEST(Compiler, int128BitwiseNot) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 z;
            __int128 n;
            set_words(&z, 0, 0);
            n = -1;
            print_words(~z);
            printf(" ");
            print_words(~n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1 0 0");
}

TEST(Compiler, int128Negate) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 one;
            __int128 wide;
            set_words(&one, 1, 0);
            set_words(&wide, 1, 1);
            print_words(-one);
            printf(" ");
            print_words(-wide);
            printf(" ");
            print_words(-(-one));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1 -1 -2 1 0");
}

TEST(Compiler, int128Equality) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            __int128 c;
            set_words(&a, 42, 1);
            set_words(&b, 42, 1);
            set_words(&c, 42, 2);
            printf("%d %d %d %d",
                (int)(a == b), (int)(a != b), (int)(a == c), (int)(a != c));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 0 1");
}

TEST(Compiler, int128SignedCompare) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 neg;
            __int128 z;
            __int128 one;
            __int128 wide;
            neg = -1;
            set_words(&z, 0, 0);
            set_words(&one, 1, 0);
            set_words(&wide, 0, 1);
            printf("%d %d %d %d %d %d",
                (int)(neg < z), (int)(neg < one), (int)(z < one),
                (int)(wide > one), (int)(neg > wide), (int)(neg <= neg));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1 1 0 1");
}

TEST(Compiler, int128UnsignedCompare) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            unsigned __int128 n;
            unsigned __int128 z;
            unsigned __int128 one;
            n = (unsigned __int128)-1;
            set_words((__int128 *)&z, 0, 0);
            set_words((__int128 *)&one, 1, 0);
            printf("%d %d %d",
                (int)(n > z), (int)(n > one), (int)(z < one));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1");
}

TEST(Compiler, int128CompareLowWordAfterEqualHigh) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 1, 1);
            set_words(&b, 2, 1);
            printf("%d %d", (int)(a < b), (int)(b > a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, int128LogicalNot) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 z;
            __int128 one;
            __int128 wide;
            set_words(&z, 0, 0);
            set_words(&one, 1, 0);
            set_words(&wide, 0, 1);
            printf("%d %d %d", (int)!z, (int)!one, (int)!wide);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 0");
}

TEST(Compiler, int128CompareToZeroLiteral) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 z;
            __int128 wide;
            set_words(&z, 0, 0);
            set_words(&wide, 0, 1);
            printf("%d %d", (int)(z == 0), (int)(wide == 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, int128CompoundAddSub) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 one;
            set_words(&a, 0, 0);
            a = a - 1;
            set_words(&one, 1, 0);
            a += one;
            print_words(a);
            printf(" ");
            a -= one;
            print_words(a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 -1 -1");
}

TEST(Compiler, int128CompoundAssignWidensIntLiteral) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 0, 0);
            a -= 1;
            print_words(a);
            printf(" ");
            a += 1;
            print_words(a);
            printf(" ");
            a &= 1;
            print_words(a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1 0 0 0 0");
}

TEST(Compiler, int128ShiftLeft) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 1, 0);
            print_words(a << 0);
            printf(" ");
            print_words(a << 1);
            printf(" ");
            set_words(&a, 1, 1);
            print_words(a << 1);
            printf(" ");
            set_words(&a, 1, 0);
            print_words(a << 64);
            printf(" ");
            print_words(a << 65);
            printf(" ");
            set_words(&a, 2, 0);
            print_words(a << 63);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 2 0 2 2 0 1 0 2 0 1");
}

TEST(Compiler, int128ShiftRightSigned) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 2, 2);
            print_words(a >> 1);
            printf(" ");
            set_words(&a, 0, 1);
            print_words(a >> 64);
            printf(" ");
            a = -1;
            print_words(a >> 1);
            printf(" ");
            print_words(a >> 64);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1 0 -1 -1 -1 -1");
}

TEST(Compiler, int128ShiftRightUnsigned) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            unsigned __int128 n;
            unsigned __int128 r;
            unsigned long *p;
            n = (unsigned __int128)-1;
            r = n >> 1;
            p = (unsigned long *)&r;
            printf("%d %d ", (int)(p[0] == (unsigned long)-1), (int)((p[1] >> 63) == 0));
            r = n >> 64;
            print_words(*(__int128 *)&r);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 -1 0");
}

TEST(Compiler, int128CompoundShift) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 1, 0);
            a <<= 64;
            print_words(a);
            printf(" ");
            a >>= 64;
            print_words(a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 1 0");
}

TEST(Compiler, int128ShiftByCharCount) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            char c;
            set_words(&a, 1, 0);
            c = 64;
            print_words(a << c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

} // namespace
