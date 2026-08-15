#include "TestFixtures.h"

namespace {

TEST(Compiler, longVariableArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            long a;
            long b;
            a = 100;
            b = 23;
            printf("%d", a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("123");
}

TEST(Compiler, unsignedVariableArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            unsigned a;
            unsigned b;
            a = 10;
            b = 7;
            printf("%d", a - b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, shortVariableArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            short a;
            short b;
            a = 4;
            b = 5;
            printf("%d", a * b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, signedVariableArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            signed a;
            a = -3;
            printf("%d", -a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, longFunctionParameterAndReturn) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        long add(long x, long y) {
            return x + y;
        }
        int main() {
            printf("%d", add(40, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, unsignedPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            unsigned v;
            unsigned* p;
            v = 99;
            p = &v;
            *p = 11;
            printf("%d", v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}

TEST(Compiler, unsignedIntIsUnsigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            unsigned int a;
            a = 5;
            printf("%d", a + 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, sizeofLongAndShort) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)sizeof(long), (int)sizeof(short), (int)sizeof(unsigned));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 2 4");
}

TEST(Compiler, multiWordUnsignedLongLocal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            unsigned long x;
            x = 41;
            printf("%d", x + 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, sizeofLongUnsignedOrderIndependent) {
    // type_name combine must not drop long when keywords are reordered.
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d %d", (int)sizeof(long unsigned), (int)sizeof(long unsigned int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 8");
}

TEST(Compiler, wideUnsignedIntegerLiteralsAssemble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long hi;
            unsigned long all;
            hi = 0xff00000000000000ULL;
            all = 18446744073709551615UL;
            printf("%d %d", (int)(hi == 0xff00000000000000ULL),
                (int)(all == 18446744073709551615UL));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, integerLiteralSuffixSetsWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d %d",
                (int)sizeof 1,
                (int)sizeof 1U,
                (int)sizeof 1L,
                (int)sizeof 1ULL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4 8 8");
}

TEST(Compiler, unsuffixedHexThatDoesNotFitSignedIntIsUnsigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void print_as_long(long v) {
            printf("%ld", v);
        }
        int main() {
            print_as_long(0xffffffff);
            printf(" %d", 0xffffffff > 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967295 1");
}

TEST(Compiler, unsuffixedHexHighBitIsUnsignedInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void print_as_long(long v) {
            printf("%ld", v);
        }
        int main() {
            print_as_long(0x80000000);
            printf(" %d %d", 0x80000000 > 0, (int)sizeof 0x80000000);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2147483648 1 4");
}

TEST(Compiler, unsuffixedDecimalThatDoesNotFitSignedIntIsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void print_as_long(long v) {
            printf("%ld", v);
        }
        int main() {
            print_as_long(2147483648);
            printf(" %d", (int)sizeof 2147483648);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2147483648 8");
}

TEST(Compiler, unsuffixedHexThatDoesNotFitUnsignedIntIsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void print_as_long(long v) {
            printf("%ld", v);
        }
        int main() {
            print_as_long(0x100000000);
            printf(" %d", (int)sizeof 0x100000000);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967296 8");
}

TEST(Compiler, unsuffixedHexThatDoesNotFitSignedLongIsUnsignedLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d", (int)sizeof 0xffffffffffffffff, 0xffffffffffffffff > 0L);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 1");
}

TEST(Compiler, suffixedHexStaysUnsignedInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void print_as_long(long v) {
            printf("%ld", v);
        }
        int main() {
            print_as_long(0xffffffffu);
            printf(" %d %d", 0xffffffffu > 0, (int)sizeof 0xffffffffu);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967295 1 4");
}

TEST(Compiler, hexLongSuffixThatDoesNotFitSignedLongIsUnsigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d", (int)sizeof 0xffffffffffffffffL, 0xffffffffffffffffL > 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 1");
}

TEST(Compiler, staticUnsignedLongPreservesUnsignedIntWrap) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static unsigned long x = 1u - 2;
        int main() {
            printf("%d %d", x == 4294967295UL, x == 18446744073709551615UL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, staticUnsignedLogicalRightShift) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static unsigned u = ~0u >> 1;
        int main() {
            printf("%d %d", u == 2147483647u, u == 4294967295u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, staticInitCastsOfUnsuffixedHex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long y = (long)0xffffffff;
        static int z = (int)0xffffffff;
        int main() {
            printf("%ld %d", y, z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967295 -1");
}

TEST(Compiler, staticInitCompareUnsuffixedHex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int s = 0xffffffff > 0;
        int main() {
            printf("%d", s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, enumeratorCopiesUnsuffixedHex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 0xffffffff, B = A };
        int main() {
            printf("%ld %ld", (long)A, (long)B);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967295 4294967295");
}

TEST(Compiler, enumeratorUnsuffixedHexDividesAsUnsigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 0xffffffff, B = A / 2 };
        int main() {
            printf("%ld %ld", (long)A, (long)B);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967295 2147483647");
}

TEST(Compiler, enumeratorUnsignedLongMaxDividesAsUnsigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 0xffffffffffffffffUL, B = A / 2 };
        int main() {
            printf("%d %d", A == 18446744073709551615UL, B == 9223372036854775807UL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, enumeratorUnsignedLongMaxKeepsIceType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 0xffffffffffffffffUL };
        int main() {
            printf("%d", (int)sizeof A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, mixedSignedAndUnsignedEnumIsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum E { A = -1, B = 0xffffffffU };
        enum F { C = 0xffffffff, D = -1 };
        int main() {
            printf("%d %d", (int)sizeof(enum E), (int)sizeof(enum F));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 8");
}

TEST(Compiler, enumeratorUnsignedLongMaxTypesTheEnum) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum E { A = 0xffffffffffffffffUL };
        int main() {
            printf("%d %d", (int)sizeof(enum E), (int)sizeof A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 8");
}

TEST(Compiler, implicitEnumeratorAfterUnsignedLongMax) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = 0xffffffffffffffffUL, B };
        int main() {
            __int128 b;
            unsigned long *p;
            b = B;
            p = (unsigned long *)&b;
            printf("%d %d %d", A == 18446744073709551615UL, (int)p[0], (int)p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 1");
}

TEST(Compiler, staticInitRelationalLeGe) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int a = 1u <= 2u;
        static int b = 3 >= 3;
        static int c = 0xffffffff >= 0;
        int main() {
            printf("%d %d %d", a, b, c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1");
}

TEST(Compiler, staticInitModuloSignedAndUnsigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int a = 7 % 3;
        static unsigned u = 0xffffffffu % 3u;
        int main() {
            printf("%d %u", a, u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, staticInitBitwiseXor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static unsigned u = 0xffu ^ 0x0fu;
        int main() {
            printf("%u", u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("240");
}

TEST(Compiler, unsuffixedDecimalThatDoesNotFitSignedLongIsInt128) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", (int)sizeof 9223372036854775808);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16");
}

TEST(Compiler, staticInitInt128CastFold) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int s = ((__int128)1 << 64) > 0;
        static int z = !((__int128)0);
        int main() {
            printf("%d %d", s, z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, enumeratorInt128ValueEmittedAtRuntime) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = (__int128)1 << 64 };
        int main() {
            __int128 a;
            unsigned long *p;
            a = A;
            p = (unsigned long *)&a;
            printf("%d %d", (int)p[0], (int)p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, implicitEnumeratorAfterInt128Shift) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { A = (__int128)1 << 64, B };
        int main() {
            __int128 a;
            __int128 b;
            unsigned long *pa;
            unsigned long *pb;
            a = A;
            b = B;
            pa = (unsigned long *)&a;
            pb = (unsigned long *)&b;
            printf("%d %d %d %d", (int)pa[0], (int)pa[1], (int)pb[0], (int)pb[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 1 1");
}

TEST(Compiler, enumeratorInt128TypesTheEnum) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum E { A = (__int128)1 << 64 };
        int main() {
            printf("%d %d", (int)sizeof(enum E), (int)sizeof A);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16 16");
}

TEST(Compiler, staticInitCastToBool) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int a = (_Bool)2;
        static int b = (_Bool)0;
        int main() {
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

} // namespace
