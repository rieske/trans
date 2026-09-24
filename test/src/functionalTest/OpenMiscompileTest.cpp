#include "TestFixtures.h"

namespace {

// C pointer subtraction has type ptrdiff_t (long). A 32-bit result truncates
// a difference that does not fit in int, and sizeof of the expression is 4.
TEST(Compiler, pointerDifferenceIsPtrdiff) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[8];
            int *p;
            int *q;
            char *lo;
            char *hi;
            p = a;
            q = a + 5;
            lo = (char *)0;
            hi = (char *)0x100000000;
            printf("%d %ld %ld", (int)sizeof(q - p), (long)(q - p), (long)(hi - lo));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 5 4294967296");
}

// Ordered floating compare. Integer cmp of the bits makes -1.0f > 1.0f and
// makes -0.0 unequal to +0.0.
TEST(Compiler, floatingCompareIsOrdered) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            float neg;
            float pos;
            double mz;
            double pz;
            neg = -1.0f;
            pos = 1.0f;
            mz = -0.0;
            pz = 0.0;
            printf("%d %d %d", neg > pos, neg < pos, mz == pz);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 1");
}

// A NaN is unordered against every relation, including itself. A positive
// number or infinity still matches the first three results.
TEST(Compiler, floatingCompareIsUnorderedForNaN) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            double n;
            double z;
            n = 0.0 / 0.0;
            z = 0.0;
            printf("%d %d %d %d %d %d", n == z, n < z, n != z, n > z, n <= z, n == n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 1 0 0 0");
}

// long double compares with fucomip. A NaN sets CF as well as PF, so a
// below jump without a parity skip treats the NaN as less than a number.
TEST(Compiler, longDoubleCompareIsUnorderedForNaN) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double n;
            long double one;
            n = 0.0L / 0.0L;
            one = 1.0L;
            printf("%d %d %d %d %d %d", n == one, n != one, n < one, n > one,
                n <= one, n >= one);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 0 0 0 0");
}

// Unsigned 64-bit values at or above 2^63 convert to a positive double.
TEST(Compiler, unsignedLongToDoubleIsPositive) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long u;
            double d;
            u = 0x8000000000000000ul;
            d = (double)u;
            printf("%d", d > 0.0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// cvtsi2sd of the signed pattern, then adding 2^64, rounds twice.
// 2^63 is exact, so the positive check above does not catch this.
TEST(Compiler, unsignedLongToDoubleRoundsOnce) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            union { double d; unsigned long long u; } x;
            unsigned long long n;
            n = 0x8000000000000401ull;
            x.d = (double)n;
            printf("%llx", x.u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("43e0000000000001");
}

// Float32 has a 24-bit significand, so adding the float encoding of 2^64
// after cvtsi2ss rounds twice. The bits are the round-to-even result.
TEST(Compiler, unsignedLongToFloatRoundsOnce) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            union { float f; unsigned int u; } x;
            unsigned long long n;
            n = 0x8000008100000000ull;
            x.f = (float)n;
            printf("%x", x.u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5f000001");
}

// A value below 2^63 fits in a signed conversion. The high-bit shift
// sequence must not run, or 1 becomes 2.
TEST(Compiler, unsignedLongToFloatKeepsSmallValues) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            union { float f; unsigned int u; } x;
            unsigned long long n;
            n = 1ull;
            x.f = (float)n;
            printf("%x", x.u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3f800000");
}

// Seven integer parameters fill the GPRs, so the seventh is on the stack and
// the following double is in xmm0. The first variadic int is 99, not that
// named stack int.
TEST(Compiler, vaStartSkipsNamedStackArgument) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int first(int a, int b, int c, int d, int e, int f, int g, double x, ...) {
            __builtin_va_list ap;
            int v;
            __builtin_va_start(ap, x);
            v = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return v;
        }
        int main() {
            printf("%d", first(1, 2, 3, 4, 5, 6, 7, 0.0, 99));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("99");
}

// L"ab" is three wchar_t elements, 12 bytes. A narrow char array is 3.
TEST(Compiler, wideStringLiteralSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", (int)sizeof(L"ab"));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
}

// One UTF-8 source character is one wchar_t, not one unit per source byte.
TEST(Compiler, wideStringDecodesUtf8) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d", (int)sizeof(L"é"), (int)L"é"[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 233");
}

// Concatenation happens after the source character is one wchar, so the
// glued literal must not turn the UTF-8 bytes back into separate escapes.
TEST(Compiler, wideStringConcatKeepsUtf8Character) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)sizeof(L"é" "z"),
                (int)(L"é" "z")[0], (int)(L"é" "z")[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 233 122");
}

// char32_t is unsigned. The same 4-byte unit in a wchar_t literal is signed.
TEST(Compiler, char32LiteralIsUnsigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d", U"\x80000000"[0] < 0, L"\x80000000"[0] < 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

// A hex escape consumes every hex digit. Stopping at 8 leaves the last
// digit as its own wchar.
TEST(Compiler, wideHexEscapeConsumesAllDigits) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)sizeof(L"\x000000041"),
                (int)L"\x000000041"[0], (int)L"\x000000041"[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 65 0");
}

// A numeric escape is one char16_t, truncated. It is not a UTF-16 pair.
TEST(Compiler, char16HexEscapeIsOneUnit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d", (int)sizeof(u"\x10000"), (int)u"\x10000"[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 0");
}

// Concatenation re-encodes the newline. A greedy hex escape would swallow "foo".
TEST(Compiler, wideConcatDoesNotSwallowHexDigit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)sizeof(L"\n" "foo"),
                (int)L"\n" "foo"[0], (int)L"\n" "foo"[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 10 102");
}

// A wide literal is not a char pointer.
TEST(Compiler, charPointerRejectsWideString) {
    SourceProgram program{R"prg(
        int main() {
            char *p = L"ok";
            return p[0];
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// A wide literal is not a char array initializer.
TEST(Compiler, charArrayRejectsWideString) {
    SourceProgram program{R"prg(
        int main() {
            char s[] = L"ab";
            return s[0];
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// Unsigned __int128 is wider than one register. The low word of 2^64 and
// of 2^127 is 0, so converting that word yields 0.
TEST(Compiler, unsignedInt128ToDoubleUsesFullWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            union { double d; unsigned long long u; } x;
            unsigned __int128 a;
            unsigned __int128 b;
            a = (unsigned __int128)1 << 64;
            b = (unsigned __int128)1 << 127;
            x.d = (double)a;
            printf("%llx", x.u);
            x.d = (double)b;
            printf(" %llx", x.u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("43f0000000000000 47e0000000000000");
}

// u8 is a narrow prefix. Gluing it under L decodes the letters of the prefix.
TEST(Compiler, wideConcatRejectsU8Prefix) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", (int)sizeof(L"a" u8"b"));
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("concatenation");
}

// L, u, and U are different wide prefixes.
TEST(Compiler, wideConcatRejectsMixedWidePrefixes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", (int)sizeof(u"a" L"b"));
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("concatenation");
}

// A wide literal gives an unsized array its element bound.
TEST(Compiler, wideStringInitializesUnsizedIntArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int s[] = L"ab";
            printf("%d %d %d %d", (int)sizeof(s), s[0], s[1], s[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 97 98 0");
}

// A complete array is filled with the code units, not the literal's address.
TEST(Compiler, wideStringInitializesSizedIntArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int t[3] = L"ab";
            printf("%d %d %d %d", (int)sizeof(t), t[0], t[1], t[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 97 98 0");
}

// One set of braces around a wide literal is still that literal, not a pointer.
TEST(Compiler, bracedWideStringInitializesIntArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int t[3] = { L"ab" };
            int s[] = { L"ab" };
            struct { int t[3]; } m = { { L"ab" } };
            printf("%d %d %d %d %d %d %d %d %d %d %d",
                (int)sizeof(t), t[0], t[1], t[2],
                (int)sizeof(s), s[0], s[1], s[2],
                m.t[0], m.t[1], m.t[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 97 98 0 12 97 98 0 97 98 0");
}

// A compound literal is sized before the wide literal is expanded, so
// { L"ab" } is counted as one element.
TEST(Compiler, compoundLiteralWideStringHasElementBound) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d %d", (int)sizeof((int[]){ L"ab" }),
                ((int[]){ L"ab" })[0], ((int[]){ L"ab" })[1], ((int[]){ L"ab" })[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 97 98 0");
}

// Braces do not make a wide literal a valid char-array initializer.
TEST(Compiler, bracedCharArrayRejectsWideString) {
    SourceProgram program{R"prg(
        int main() {
            char s[4] = { L"ab" };
            return s[0];
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// A wide literal initializes the subobject array, not the outer array.
TEST(Compiler, wideStringInitializesNestedIntRow) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[2][3] = { L"ab" };
            printf("%d %d %d %d %d %d",
                a[0][0], a[0][1], a[0][2], a[1][0], a[1][1], a[1][2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("97 98 0 0 0 0");
}

// A bare wide literal does not initialize an array of arrays. The braced
// form above does. Without braces this is an invalid initializer.
TEST(Compiler, bareWideStringRejectsArrayOfArrays) {
    SourceProgram program{R"prg(
        int main() {
            int a[2][3] = L"ab";
            return a[0][0];
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// A wide literal initializes the member array with code units.
TEST(Compiler, wideStringInitializesStructMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            struct { int t[3]; } s = { L"ab" };
            printf("%d %d %d", s.t[0], s.t[1], s.t[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("97 98 0");
}

// A wide literal is not a character-array member.
TEST(Compiler, charMemberRejectsWideString) {
    SourceProgram program{R"prg(
        int main() {
            struct { char s[4]; } c = { L"ab" };
            return c.s[0];
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// Any wide literal in the concatenation makes the result wide.
TEST(Compiler, wideConcatPrefixOnLaterToken) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d %d %d %d",
                (int)sizeof("a" L"b"), (int)("a" L"b")[0], (int)("a" L"b")[1],
                (int)sizeof("a" u"b"), (int)("a" u"b")[0], (int)("a" u"b")[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 97 98 6 97 98");
}

// A character constant is an int, and plain char is signed, so '\xff' is -1.
TEST(Compiler, charConstantSignExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d", '\xff', '\377');
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

// A decimal that fits in no standard integer and not in signed __int128 is
// not an int. gcc gives it an extended 16-byte type.
TEST(Compiler, oversizedIntegerLiteralIsNotInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            __int128 v = 9999999999999999999999999999999999999999;
            unsigned long long *p = (unsigned long long *)&v;
            printf("%d %llu %llu", (int)sizeof(v), p[1], p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16 7145508105175220139 13399722918938673151");
}

// Both constants are 2^127 after wrapping. Hex keeps unsigned __int128.
// The same residue as a decimal stays signed.
TEST(Compiler, oversizedHexLiteralStaysUnsigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d",
                0x180000000000000000000000000000000 > 0,
                510423550381407695195061911147652317184 > 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, wideNamedEscapesAreCodeUnits) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d %d %d %d %d %d %d %d",
                (int)L"\t"[0], (int)L"\r"[0], (int)L"\a"[0], (int)L"\b"[0],
                (int)L"\f"[0], (int)L"\v"[0], (int)L"\\"[0], (int)L"\'"[0],
                (int)L"\""[0], (int)L"\?"[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 13 7 8 12 11 92 39 34 63");
}

TEST(Compiler, wideOctalAndShortUniversalEscape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d", (int)L"\101"[0], (int)L"\u0041"[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65 65");
}

TEST(Compiler, wideUtf8AboveAscii) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d %d", (int)sizeof(L"€"), (int)L"€"[0],
                (int)sizeof(L"😀"), (int)L"😀"[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 8364 8 128512");
}

TEST(Compiler, char16EmojiIsSurrogatePair) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)sizeof(u"😀"), (int)u"😀"[0], (int)u"😀"[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6 55357 56832");
}

TEST(Compiler, narrowConcatReencodesControls) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char *tab = "\t" "a";
            char *raw = "\x01" "a";
            printf("%d %d %d %d", tab[0], tab[1], raw[0], raw[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 97 1 97");
}

TEST(Compiler, unsignedInt128ToFloatUsesFullWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            union { float f; unsigned u; } x;
            unsigned __int128 a;
            unsigned __int128 b;
            unsigned __int128 c;
            a = 1;
            b = (unsigned __int128)1 << 64;
            c = (unsigned __int128)1 << 127;
            x.f = (float)a;
            printf("%x", x.u);
            x.f = (float)b;
            printf(" %x", x.u);
            x.f = (float)c;
            printf(" %x", x.u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3f800000 5f800000 7f000000");
}

TEST(Compiler, wideStringDropsOnlyTheTerminator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int t[2] = L"ab";
            struct { int t[2]; } s = { L"ab" };
            printf("%d %d %d %d", t[0], t[1], s.t[0], s.t[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("97 98 97 98");
}

TEST(Compiler, wideStringExcessIsAnError) {
    SourceProgram program{R"prg(
        int main() {
            int t[1] = L"ab";
            return t[0];
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements");
}

TEST(Compiler, wideStringMemberExcessIsAnError) {
    SourceProgram program{R"prg(
        int main() {
            struct { int t[1]; } s = { L"ab" };
            return s.t[0];
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements");
}

TEST(Compiler, compoundLiteralKeepsSeveralElements) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)sizeof((int[]){ 1, 2 }),
                ((int[]){ 1, 2 })[0], ((int[]){ 1, 2 })[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 1 2");
}

TEST(Compiler, compoundLiteralArrayOfArraysKeepsRow) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int row[][3] = { L"ab" };
            printf("%d %d %d %d", (int)sizeof(row), row[0][0], row[0][1], row[0][2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 97 98 0");
}

TEST(Compiler, compoundLiteralCharRejectsWideString) {
    SourceProgram program{R"prg(
        int main() {
            return ((char[]){ L"ab" })[0];
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

TEST(Compiler, char32LiteralAboveSignedInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned int w[] = U"\x80000000";
            printf("%d %u", (int)sizeof(w), w[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 2147483648");
}

} // namespace
