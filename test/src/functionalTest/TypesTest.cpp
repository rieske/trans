#include "TestFixtures.h"

namespace {

TEST(Compiler, longVariableArithmetic) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

// Floating constants must parse; cast to int truncates toward zero.
TEST(Compiler, floatConstantCastToInt) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d %d", (int)3.7, (int)0.5);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 0");
}

// khash-style load factor: integer * double + double, cast to int (round).
TEST(Compiler, floatLoadFactorRound) {
    SourceProgram program{R"prg(
        static const double upper = 0.77;
        int main() {
            unsigned n;
            n = 10;
            printf("%d", (int)(n * upper + 0.5));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

// C floating constants with exponents (git test-date: getnanotime() / 1.0e9).
TEST(Compiler, scientificFloatDivision) {
    SourceProgram program{R"prg(
        int main() {
            double seconds;
            seconds = 1000000000.0 / 1.0e9;
            printf("%d", (int)seconds);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, scientificFloatVariants) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d %d %d", (int)1e3, (int)2.5e1, (int)1.0E-1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1000 25 0");
}

// SysV AMD64: floating args must go in xmm0.. and AL must count them for
// variadic callees (printf). Passing double bits in rsi with al=0 yields 0.00
// (git json-writer jw_object_double / t0019).
TEST(Compiler, printfDoubleUsesXmmAndAl) {
    SourceProgram program{R"prg(
        int main() {
            double d;
            d = 3.14;
            printf("%.2f", d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3.14");
}

// Non-variadic double parameter: caller xmm0 -> callee reads xmm0
// (git jw_object_double(jw, key, precision, value)).
TEST(Compiler, doubleParameterRoundTrip) {
    SourceProgram program{R"prg(
        int as_int(double d) {
            return (int)d;
        }
        int main() {
            printf("%d %d", as_int(3.9), as_int(100.1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 100");
}

// Implicit float->int conversion on return (git similarity_index:
// return p->score * 100 / MAX_SCORE with MAX_SCORE = 60000.0).
// Assignment path already emits cvttsd2si; bare return must as well.
TEST(Compiler, implicitDoubleToIntOnReturn) {
    SourceProgram program{R"prg(
        int similarity(int score) {
            return score * 100 / 60000.0;
        }
        int main() {
            printf("%d %d", similarity(60000), similarity(30000));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("100 50");
}

// Implicit float->int conversion on call arguments (C 6.5.2.2).
// Assignment `int s = 60000.0` already converts; passing the double as an int
// parameter must cvttsd2si too. Without it, git record_rename_pair(..., MAX_SCORE)
// with MAX_SCORE=60000.0 stores score 0 and empty-file renames print "0%".
TEST(Compiler, implicitDoubleToIntOnCallArg) {
    SourceProgram program{R"prg(
        void record(int *p, int score) {
            *p = score;
        }
        int similarity(int score) {
            return score * 100 / 60000.0;
        }
        int main() {
            int s;
            record(&s, 60000.0);
            printf("%d %d", s, similarity(s));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("60000 100");
}

// Implicit float->int on member assignment through an lvalue address
// (git wt-status: d->rename_score = p->score * 100 / MAX_SCORE).
// Plain `int x = double_expr` converts via Assign; member store used LvalueAssign
// of the raw double bits first (IEEE 100.0 low 32 bits are 0 -> R0), then
// converted only into the expression temp. Convert before the store.
TEST(Compiler, implicitDoubleToIntOnMemberAssign) {
    SourceProgram program{R"prg(
        struct pair {
            unsigned short score;
        };
        struct data {
            int rename_score;
        };
        int main() {
            struct pair p;
            struct data d;
            p.score = 60000;
            d.rename_score = p.score * 100 / 60000.0;
            printf("%d %d", (int)p.score, d.rename_score);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("60000 100");
}

// Same conversion when storing through a pointer (*p = double_expr).
TEST(Compiler, implicitDoubleToIntOnPointerAssign) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            int *p;
            p = &x;
            *p = 60000 * 100 / 60000.0;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("100");
}

// Usual arithmetic conversions on relational ops (C 6.3.1.8 / 6.5.8):
// when one side is double, integer literals must become double before compare.
// Integer cmp of double bits vs bare 1 rejects every normal positive double
// (git: !(0 < sample_rate && sample_rate <= 1) always warned).
TEST(Compiler, doubleCompareWithIntegerLiteral) {
    SourceProgram program{R"prg(
        int main() {
            double x;
            x = 0.5;
            printf("%d %d %d", 0 < x, x <= 1, x <= 1.0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1");
}

// git pseudo-merge sampleRate bounds: !(0 < rate && rate <= 1) must be false
// for rates in (0, 1].
TEST(Compiler, doubleSampleRateBoundsCheck) {
    SourceProgram program{R"prg(
        struct group {
            double sample_rate;
        };
        int out_of_range(double rate) {
            return !(0 < rate && rate <= 1);
        }
        int main() {
            struct group g;
            g.sample_rate = 0.5;
            printf("%d %d %d %d",
                   out_of_range(0.5),
                   out_of_range(1.0),
                   out_of_range(0.25),
                   out_of_range(g.sample_rate));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 0 0");
}

// Reject values outside (0, 1] the same way git does.
TEST(Compiler, doubleSampleRateOutOfRange) {
    SourceProgram program{R"prg(
        int out_of_range(double rate) {
            return !(0 < rate && rate <= 1);
        }
        int main() {
            printf("%d %d %d",
                   out_of_range(0.0),
                   out_of_range(1.5),
                   out_of_range(-0.1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1");
}

// Implicit integer widening on return (C 6.8.6.4): uint32_t / ntohl result
// returned as long/off_t must zero-extend. Without it, a 32-bit store + 64-bit
// load leaves stack garbage in the high half; git nth_packed_object_offset
// returns offsets like 0x7fff0000033b and cat-file dies with
// "offset beyond end of packfile" on index v1 packs.
TEST(Compiler, implicitUint32ToLongOnReturn) {
    SourceProgram program{R"prg(
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

// Same conversion when the double is a parameter (not only arithmetic result).
// Avoid unary-minus on float literals here (separate bug: integer neg of IEEE bits).
TEST(Compiler, implicitDoubleParamToIntOnReturn) {
    SourceProgram program{R"prg(
        int truncate(double d) {
            return d;
        }
        int main() {
            double neg;
            neg = 0.0 - 1.1;
            printf("%d %d", truncate(3.9), truncate(neg));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 -1");
}

// Unary minus on a double must flip the IEEE sign bit, not integer-neg the bit pattern.
TEST(Compiler, unaryMinusDoubleFlipsSignBit) {
    SourceProgram program{R"prg(
        int main() {
            double d;
            d = -1.5;
            printf("%.1f %d", d, (int)d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1.5 -1");
}

// Mixed integer + floating register args (git jw_object_double signature shape).
TEST(Compiler, mixedIntAndDoubleArgs) {
    SourceProgram program{R"prg(
        int scale(int n, double f) {
            return (int)(n * f);
        }
        int main() {
            printf("%d", scale(10, 0.77));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

// SysV: floating return values are in xmm0, not rax (git strtod / atof).
TEST(Compiler, doubleReturnInXmm0) {
    SourceProgram program{R"prg(
        double half(double x) {
            return x / 2.0;
        }
        int main() {
            double d;
            d = half(6.5);
            printf("%.1f", d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3.2");
}

// Libc strtod returns double in xmm0 (git test-json-writer get_d).
TEST(Compiler, strtodReturnsDouble) {
    SourceProgram program{R"prg(
        double strtod(const char *nptr, char **endptr);
        int main() {
            char *end;
            double d;
            d = strtod("3.140", &end);
            printf("%.2f", d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3.14");
}

// C99 6.4.4.1: unsuffixed hex that does not fit in signed int is unsigned int,
// so conversion to long long zero-extends (git jw_object_intmax(..., 0xffffffff)).
TEST(Compiler, hexConstantFitsUnsignedIntNotSigned) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
        int main() {
            printf("%d %d", (int)sizeof(short), (int)sizeof(unsigned short));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 2");
}

TEST(Compiler, unsignedShortPointerIndexStrideTwo) {
    SourceProgram program{R"prg(
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
    program.compileWithPreprocess();
    program.runAndExpect("1 1 0 0");
}

// Locals pad int to an 8-byte stack slot; stores through int* write only 4 bytes.
// Loading/comparing the plain local must use a 32-bit access (zero/sign extend),
// else dirty high bits make type != 1 fail and type_name(type) return NULL
// (git: peel_object_ext *typep = o->type; then type != OBJ_COMMIT).
TEST(Compiler, intLocalSetViaPointerIgnoresHighStackBits) {
    SourceProgram program{R"prg(
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

// Double parameter/return through xmm0 (git jw_object_double-style).
TEST(Compiler, doubleParameterAndReturnRoundTrip) {
    SourceProgram program{R"prg(
        double twice(double x) {
            return x + x;
        }
        int main() {
            double d;
            d = twice(1.5);
            printf("%d", (int)d);
            return 0;
        }
    )prg", "double_roundtrip"};
    program.compile();
    program.runAndExpect("3");
}

// 32-bit float constants convert for int cast; full float32 arithmetic is thin.
TEST(Compiler, floatConstantUsedInDoubleArithmetic) {
    SourceProgram program{R"prg(
        int main() {
            double d;
            d = 1.5f + 1.5f;
            printf("%d", (int)d);
            return 0;
        }
    )prg", "float_const_in_double"};
    program.compile();
    program.runAndExpect("3");
}

// Mixed float/double arithmetic promotes and converts back for the int cast.
TEST(Compiler, mixedFloatDoubleArithmetic) {
    SourceProgram program{R"prg(
        int main() {
            float f;
            double d;
            f = 2.0f;
            d = 0.5;
            printf("%d", (int)(f / d + 1.0));
            return 0;
        }
    )prg", "float_double_mix"};
    program.compile();
    program.runAndExpect("5");
}

// Division by a double constant used in git-style score scaling.
TEST(Compiler, doubleDivisionTruncatesTowardZeroOnCast) {
    SourceProgram program{R"prg(
        int main() {
            double n;
            n = 10.0 / 3.0;
            printf("%d %d", (int)n, (int)(0.0 - 1.9));
            return 0;
        }
    )prg", "double_div_trunc"};
    program.compile();
    program.runAndExpect("3 -1");
}

} // namespace
