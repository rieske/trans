#include "TestFixtures.h"

namespace {

// C 6.3.1.1 / 6.5.7 / 6.5.3.3: << >> + - ~ promote a narrow operand to int
// and the result has that promoted type. Values below differ if the op is
// performed at the unpromoted width (char 8, short 16).
TEST(Compiler, integerPromotionsWidenUnaryAndShiftOfNarrowTypes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char c;
            unsigned char uc;
            short s;
            unsigned short us;
            unsigned char b0;
            unsigned char b1;
            unsigned char b2;
            unsigned char b3;
            unsigned pack;
            c = 1;
            uc = 1;
            s = 1;
            us = 1;
            printf("%d %d %d %d ", c << 8, uc << 8, s << 16, us << 16);
            c = 0x54;
            printf("%d ", (c << 24) == 0x54000000);
            b0 = 0x54;
            b1 = 0x52;
            b2 = 0x45;
            b3 = 0x45;
            pack = (unsigned)((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
            printf("%d ", pack == 0x54524545u);
            uc = 128;
            printf("%d %d ", +uc, (int)sizeof(+uc));
            uc = 0;
            us = 0;
            printf("%d %d ", ~uc, ~us);
            uc = 1;
            us = 1;
            printf("%d %d", -uc, -us);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("256 256 65536 65536 1 1 128 4 -1 -1 -1 -1");
}

// Passing a negative signed char to unsigned char must zero-extend before
// the usual arithmetic conversions. Inlining the call used to keep the
// signed value, so 210 / (uint8_t)-46 became 210 / -46.
TEST(Compiler, unsignedCharParameterZeroExtendsSignedCharArgument) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned char divu8(unsigned char a, unsigned char b) {
            return a / b;
        }

        int main() {
            signed char g;
            g = -46;
            printf("%u %u", (unsigned)divu8(210, g), (unsigned)divu8(-1, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 127");
}

// The value of a signed bit-field assignment is the field after it is stored,
// sign-extended to int. A later read of the same field already did that.
TEST(Compiler, signedBitFieldAssignmentSignExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { signed f0 : 25; };

        int main() {
            struct S s;
            int v;
            v = (s.f0 = 0x012C2E8C);
            printf("%d %d", v, (int)s.f0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-13881716 -13881716");
}

// An unsigned bit-field that fits in int promotes to int (C 6.3.1.1).
// Comparing it as unsigned makes -2 > 5 true.
TEST(Compiler, unsignedBitFieldPromotesToInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        union U { unsigned f0 : 3; };
        struct S { unsigned f0 : 32; };

        int main() {
            union U u;
            struct S s;
            u.f0 = 5;
            s.f0 = 5;
            printf("%d %d %d", -2 > u.f0, -2 > (int)u.f0, -2 > s.f0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 1");
}

// Promotion changes the value type, not the container width. A narrow field
// must not be loaded or stored as a 4-byte int, and a field wider than int
// stays that width.
TEST(Compiler, bitFieldPromotionKeepsContainerWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct C { unsigned char a:3; unsigned char b; };
        struct B { _Bool f:1; char n; };
        struct L { unsigned long f:32; unsigned long hi; };
        struct W { signed long long f:40; unsigned char tail; };

        int main() {
            struct C c;
            struct B b;
            struct L l;
            struct W w;
            c.b = 0xAB;
            c.a = 5;
            b.n = 0x11;
            b.f = 2;
            l.hi = 0x111111111UL;
            l.f = 5;
            w.tail = 0x22;
            w.f = ((signed long long)1 << 32) | 7;
            printf("%u %u %d %u %llu %llu %lld %u %d %d",
                (unsigned)c.a, (unsigned)c.b,
                (int)b.f, (unsigned char)b.n,
                (unsigned long long)l.f, (unsigned long long)l.hi,
                (long long)w.f, (unsigned)w.tail,
                -2 > c.a, -2 > l.f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 171 1 17 5 4581298449 4294967303 34 0 1");
}

// Assignment, ++/--, and comma keep the declared type of a narrow bit-field.
// Integer promotion applies to the value, not to sizeof of those expressions.
TEST(Compiler, narrowBitFieldExpressionsKeepDeclaredSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct C { unsigned char a:3; short s:9; _Bool b:1; };

        int main() {
            struct C c;
            printf("%zu %zu %zu %zu %zu %zu",
                sizeof(c.a = 1), sizeof(c.s = 1), sizeof(c.b = 1),
                sizeof(c.a++), sizeof(++c.a), sizeof(0, c.a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 1 1 1 1");
}

// Postfix keeps the promoted rvalue, so an unsigned bit-field compares as int.
// A comma expression keeps the decayed pointer type of an array or function.
TEST(Compiler, postfixBitFieldPromotesAndCommaDecays) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(void) { return 1; }

        int main() {
            union U { unsigned f0:3; };
            union U u;
            int a[4];
            int cmp;
            int post;
            u.f0 = 5;
            cmp = -2 > u.f0;
            post = -2 > u.f0++;
            printf("%d %d %zu %zu %zu", cmp, post,
                sizeof(0, a), sizeof(0, "ab"), sizeof(0, f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 8 8 8");
}

// _Generic and a statement expression must keep the declared bit-field type.
// Otherwise a store through the forward uses int width and clobbers the next member.
TEST(Compiler, genericBitFieldKeepsDeclaredWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            struct C { unsigned char a:3; unsigned char b; };
            struct C c;
            unsigned char afterAssign;
            c.b = 0xAB;
            (_Generic(0, default: c.a)) = 5;
            afterAssign = c.b;
            c.b = 0xAB;
            ++_Generic(0, default: c.a);
            printf("%u %u %u %zu %zu",
                (unsigned)c.a, (unsigned)afterAssign, (unsigned)c.b,
                sizeof(_Generic(0, default: c.a = 1)),
                sizeof(({ c.a = 1; })));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6 171 171 1 1");
}

// (signed char)-32 promoted to int is -32, not the zero-extended byte 224.
// ~((int)(signed char)28) is -29, which is greater than -32.
TEST(Compiler, signedCharMaxSignExtendsAtOpt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        #define max(a, b) \
            ({ __typeof__(a) _a = (a); \
               __typeof__(b) _b = (b); \
               _a > _b ? _a : _b; })
        int main(void) {
            short v = (short) max(~((int)(signed char)28),
                (int) max((signed char)-32, (signed char)-32));
            printf("%d", (int)v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-29");
}

} // namespace
