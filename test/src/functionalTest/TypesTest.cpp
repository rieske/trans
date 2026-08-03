#include "TestFixtures.h"

namespace {

TEST(Compiler, longVariableArithmetic) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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

TEST(Compiler, typeCastIntToPointerAndBack) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int x;
            int* p;
            int y;
            x = 7;
            p = (int*)&x;
            y = *p;
            printf("%d", y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, typeCastBetweenIntegerTypes) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            long a;
            int b;
            a = 1000;
            b = (int)a;
            printf("%d", b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1000");
}

// Named bit-fields and anonymous padding appear in system headers. Product
// contract: widths are ignored (ordinary members); anonymous padding is dropped.
// See ProductContracts.bitfieldWidthsIgnoredForLayout for the frozen oracle.
TEST(Compiler, bitfieldMembersParseAndAccess) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Flags {
            unsigned a : 1;
            unsigned b : 2;
            int : 32;
            int c;
        };
        int main() {
            struct Flags f;
            f.a = 1;
            f.b = 3;
            f.c = 40;
            printf("%d %d %d %d", (int)sizeof(struct Flags), f.a, f.b, f.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 1 3 40");
}

// Implicit integer widening on return (C 6.8.6.4): uint32_t / ntohl result
// returned as long/off_t must zero-extend. Without it, a 32-bit store + 64-bit
// load leaves stack garbage in the high half; git nth_packed_object_offset
// returns offsets like 0x7fff0000033b and cat-file dies with
// "offset beyond end of packfile" on index v1 packs.
TEST(Compiler, implicitUint32ToLongOnReturn) {
    SourceProgram program{R"prg(#include <stdio.h>
        unsigned int be32(void) {
            return 827u;
        }
        long as_off(void) {
            return be32();
        }
        int main() {
            long o;
            o = be32();
            printf("%ld %ld", o, as_off());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("827 827");
}

// C99 6.4.4.1: unsuffixed hex that does not fit in signed int is unsigned int,
// so conversion to long long zero-extends (git jw_object_intmax(..., 0xffffffff)).
TEST(Compiler, hexConstantFitsUnsignedIntNotSigned) {
    SourceProgram program{R"prg(#include <stdio.h>
        void print_ll(long long v) {
            printf("%lld", v);
        }
        int main() {
            print_ll(0xffffffff);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967295");
}

// Integer to unsigned char must truncate to 8 bits (git sane_ctype / BOM index).
// (unsigned char)(-1) is 255, not all-ones as int/long.
TEST(Compiler, castToUnsignedCharTruncates) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int c;
            c = -1;
            printf("%d %u %lu", (unsigned char)c, (unsigned char)c, (unsigned long)(unsigned char)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("255 255 255");
}

// Constant cast and values outside 0..255 also narrow.
TEST(Compiler, castToUnsignedCharConstantAndLarge) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int c;
            c = 256 + 7;
            printf("%d %d", (unsigned char)-1, (unsigned char)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("255 7");
}

// Array index with (unsigned char) must use 0..255, not a huge negative offset.
TEST(Compiler, castToUnsignedCharAsArrayIndex) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int t[256];
            int c;
            int i;
            i = 0;
            while (i < 256) {
                t[i] = i;
                i = i + 1;
            }
            c = -1;
            printf("%d", t[(unsigned char)c]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("255");
}

// Signed char cast truncates and sign-extends (char)-1 stays -1 as int.
TEST(Compiler, castToSignedCharSignExtends) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int c;
            c = 255;
            printf("%d %d", (char)c, (char)-1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

// Narrowing cast to unsigned int zeros the upper 32 bits.
TEST(Compiler, castToUnsignedIntTruncates) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            long v;
            v = -1;
            printf("%lu", (unsigned long)(unsigned)v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4294967295");
}

// Libc getc returns int; EOF is -1. The 32-bit return in eax must be sign-extended
// so `c == -1` is true (git config parser empty-file / end-of-file path).
TEST(Compiler, getcEofComparesEqualToMinusOne) {
    SourceProgram program{R"prg(
        int printf(const char *, ...);
        int getc(void *f);
        void *fopen(const char *p, const char *m);
        int fclose(void *f);
        int main() {
            int c;
            void *f;
            f = fopen("/tmp/trans_empty_eof_test", "w");
            fclose(f);
            f = fopen("/tmp/trans_empty_eof_test", "r");
            c = getc(f);
            fclose(f);
            if (c == -1) {
                printf("eof");
            } else {
                printf("not %d", c);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("eof");
}

// Same for a callee we define: int returning -1 must compare equal to -1.
TEST(Compiler, intReturnMinusOneComparesEqual) {
    SourceProgram program{R"prg(#include <stdio.h>
        int ret_neg(void) {
            return -1;
        }
        int main() {
            int c;
            c = ret_neg();
            if (c == -1) {
                printf("ok");
            } else {
                printf("bad %d", c);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// short is 2 bytes (C ABI). ctype tables are unsigned short[]; isalpha uses
// (*__ctype_b_loc())[c] which must index with stride 2, not word stride 8.
TEST(Compiler, sizeofShortIsTwo) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d %d", (int)sizeof(short), (int)sizeof(unsigned short));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 2");
}

TEST(Compiler, unsignedShortPointerIndexStrideTwo) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            unsigned short t[4];
            unsigned short *p;
            t[0] = 10;
            t[1] = 20;
            t[2] = 30;
            t[3] = 40;
            p = t;
            printf("%d %d %d", (int)p[0], (int)p[1], (int)p[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 20 30");
}

// isalpha from libc must see the real ctype table (unsigned short entries).
// Needs preprocess so isalpha expands to (*__ctype_b_loc())[c] & _ISalpha.
TEST(Compiler, isalphaRecognizesLetters) {
    SourceProgram program{R"prg(
        #include <ctype.h>
        #include <stdio.h>
        int main() {
            printf("%d %d %d %d", isalpha('u') != 0, isalpha('A') != 0,
                   isalpha('[') != 0, isalpha('1') != 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 0 0");
}

// Locals pad int to an 8-byte stack slot; stores through int* write only 4 bytes.
// Loading/comparing the plain local must use a 32-bit access (zero/sign extend),
// else dirty high bits make type != 1 fail and type_name(type) return NULL
// (git: peel_object_ext *typep = o->type; then type != OBJ_COMMIT).
TEST(Compiler, intLocalSetViaPointerIgnoresHighStackBits) {
    SourceProgram program{R"prg(#include <stdio.h>
        void set_int(int *p, int v) {
            *p = v;
        }
        void dirty_slot(void *p) {
            unsigned long long *q;
            q = (unsigned long long *)p;
            *q = 0xFFFFFFFFFFFFFFFFULL;
        }
        const char *names[5];
        const char *type_name(unsigned int type) {
            if (type >= 5) {
                return 0;
            }
            return names[type];
        }
        int main() {
            int type;
            names[0] = 0;
            names[1] = "commit";
            names[2] = "tree";
            names[3] = "blob";
            names[4] = "tag";
            dirty_slot(&type);
            set_int(&type, 1);
            if (type != 1) {
                const char *n;
                n = type_name((unsigned int)type);
                if (n) {
                    printf("ne %s", n);
                } else {
                    printf("ne (null)");
                }
                return 1;
            }
            {
                const char *n;
                n = type_name((unsigned int)type);
                if (n) {
                    printf("ok %s", n);
                } else {
                    printf("ok (null)");
                }
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok commit");
}

// auto / register storage class specifiers (C89 form).
TEST(Compiler, autoAndRegisterStorageClass) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            auto int a = 20;
            register int b = 22;
            printf("%d", a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Multiple type qualifiers on a declaration.
TEST(Compiler, multipleTypeQualifiers) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            const volatile int x = 42;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Pointer with multi-qualifier list (* const volatile).
TEST(Compiler, pointerMultiQualifierList) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int x;
            int * const volatile p = &x;
            *p = 42;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

} // namespace

TEST(Compiler, unsignedIntIsUnsigned) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            unsigned int a;
            a = 5;
            printf("%d", a + 1);
            return 0;
        }
    )prg", "unsigned_int_is_unsigned"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, sizeofLongAndShort) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d %d %d", (int)sizeof(long), (int)sizeof(short), (int)sizeof(unsigned));
            return 0;
        }
    )prg", "sizeof_long_short"};
    program.compile();
    program.runAndExpect("8 2 4");
}

TEST(Compiler, multiWordUnsignedLongLocal) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            unsigned long x;
            x = 41;
            printf("%d", x + 1);
            return 0;
        }
    )prg", "multiword_ulong_local"};
    program.compile();
    program.runAndExpect("42");
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

TEST(Compiler, sizeofLongUnsignedOrderIndependent) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d", (int)sizeof(long unsigned), (int)sizeof(long unsigned int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 8");
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

