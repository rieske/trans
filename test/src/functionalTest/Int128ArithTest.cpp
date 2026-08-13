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

TEST(Compiler, int128Mul) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 3, 0);
            set_words(&b, 4, 0);
            print_words(a * b);
            printf(" ");
            set_words(&a, 0, 1);
            set_words(&b, 2, 0);
            print_words(a * b);
            printf(" ");
            a = -1;
            set_words(&b, 2, 0);
            print_words(a * b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 0 0 2 -2 -1");
}

TEST(Compiler, int128DivMod) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 12, 0);
            set_words(&b, 4, 0);
            print_words(a / b);
            printf(" ");
            print_words(a % b);
            printf(" ");
            set_words(&a, 0, 3);
            set_words(&b, 0, 1);
            print_words(a / b);
            printf(" ");
            a = -6;
            set_words(&b, 2, 0);
            print_words(a / b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 0 0 0 3 0 -3 -1");
}

TEST(Compiler, int128DivSignedVsUnsigned) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 n;
            unsigned __int128 u;
            unsigned __int128 r;
            unsigned long *p;
            n = -1;
            u = (unsigned __int128)-1;
            r = u / 2;
            p = (unsigned long *)&r;
            printf("%d %d", (int)((n / 2) == 0), (int)((p[1] >> 62) == 1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, int128CompoundMulDiv) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 0, 1);
            a *= 2;
            print_words(a);
            printf(" ");
            a /= 2;
            print_words(a);
            printf(" ");
            set_words(&a, 5, 1);
            a %= a - 5;
            print_words(a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 2 0 1 5 0");
}

TEST(Compiler, int128LiteralDecimalPow2_64) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            print_words(18446744073709551616);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, int128LiteralHexPow2_64Plus42) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            print_words(0x10000000000000000);
            printf(" ");
            print_words(0x1000000000000002a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 42 1");
}

TEST(Compiler, int128LiteralUnsignedMax) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            print_words(0xffffffffffffffffffffffffffffffff);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, int128LiteralSizeofAndAdd) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            printf("%d ", (int)sizeof 18446744073709551616);
            printf("%d ", (int)sizeof 18446744073709551616U);
            print_words(18446744073709551616 + 1);
            printf(" ");
            printf("%d", (int)(0x10000000000000000 == 18446744073709551616));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16 16 1 1 1");
}

// Control: __int128 as the first struct member already works.
TEST(Compiler, int128FirstStructMemberLayout) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            __int128 a;
            int b;
        };
        int main(void) {
            struct S s;
            s.a = 1;
            s.b = 2;
            printf("%d %d %d", (int)sizeof(struct S), (int)s.b, (int)(long)s.a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 2 1");
}

// Keyword __int128 after another member must parse (GCC/SysV layout: pad to 16, size 32).
TEST(Compiler, int128NonFirstStructMemberLayout) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            __int128 b;
        };
        int main(void) {
            struct S s;
            s.a = 3;
            s.b = 4;
            printf("%d %d %d %d", (int)sizeof(struct S), (int)__builtin_offsetof(struct S, b),
                    s.a, (int)(long)s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 16 3 4");
}

TEST(Compiler, int128AfterCharStructMemberLayout) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            char c;
            __int128 x;
        };
        int main(void) {
            struct S s;
            s.c = 2;
            s.x = 40;
            printf("%d %d %d", (int)sizeof(struct S), (int)__builtin_offsetof(struct S, x),
                    (int)(s.c + (long)s.x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 16 42");
}

// Two __int128 members: second keyword must also parse.
TEST(Compiler, int128TwoMembersInStruct) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            __int128 a;
            __int128 b;
        };
        int main(void) {
            struct S s;
            s.a = 1;
            s.b = 2;
            printf("%d %d", (int)sizeof(struct S), (int)(long)(s.a + s.b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 3");
}

TEST(Compiler, int128NonFirstUnionMemberLayout) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        union U {
            int a;
            __int128 b;
        };
        int main(void) {
            union U u;
            u.b = 7;
            printf("%d %d", (int)sizeof(union U), (int)(long)u.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16 7");
}

// Typedef form already works as a regression lock for the intended layout.
TEST(Compiler, int128NonFirstMemberViaTypedef) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef __int128 i128;
        struct S {
            int a;
            i128 b;
        };
        int main(void) {
            struct S s;
            s.a = 3;
            s.b = 4;
            printf("%d %d %d %d", (int)sizeof(struct S), (int)__builtin_offsetof(struct S, b),
                    s.a, (int)(long)s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 16 3 4");
}

} // namespace
