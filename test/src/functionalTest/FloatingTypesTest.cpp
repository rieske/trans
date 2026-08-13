#include "TestFixtures.h"

namespace {

TEST(Compiler, floatConstantCastToInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d %d", (int)3.7, (int)0.5);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 0");
}

TEST(Compiler, floatLoadFactorRound) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, scientificFloatDivision) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)1e3, (int)2.5e1, (int)1.0E-1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1000 25 0");
}

TEST(Compiler, printfDoubleUsesXmmAndAl) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, printfDoubleDoesNotDisplaceFollowingInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            double d;
            int code;
            d = 1.5;
            code = 42;
            printf("%.1f %d", d, code);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1.5 42");
}

TEST(Compiler, doubleParameterRoundTrip) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, implicitDoubleToIntOnReturn) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, implicitDoubleToIntOnCallArg) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, implicitDoubleToIntOnMemberAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, implicitDoubleToIntOnPointerAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, doubleCompareWithIntegerLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, doubleSampleRateBoundsCheck) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, doubleSampleRateOutOfRange) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, implicitDoubleParamToIntOnReturn) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, unaryMinusDoubleFlipsSignBit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, mixedIntAndDoubleArgs) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, doubleReturnInXmm0) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, strtodReturnsDouble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, doubleParameterAndReturnRoundTrip) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        double twice(double x) {
            return x + x;
        }
        int main() {
            double d;
            d = twice(1.5);
            printf("%d", (int)d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, floatConstantUsedInDoubleArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            double d;
            d = 1.5f + 1.5f;
            printf("%d", (int)d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, mixedFloatDoubleArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            float f;
            double d;
            f = 2.0f;
            d = 0.5;
            printf("%d", (int)(f / d + 1.0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

// Fuzzer: 6.0L was stored as IEEE double bits, so printf %Lf and (int) cast saw 0.
TEST(Compiler, longDoubleConstantAssignmentPrints) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double x;
            x = 6.0L;
            printf("%.0Lf", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, longDoubleConstantCastToInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double x;
            x = 6.0L;
            printf("%d", (int)x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, longDoubleAfterManyStackArgsSameTu) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int after_many(int a, int b, int c, int d, int e, int f, int g, long double ld) {
            return a + g + (int)ld;
        }
        int main() {
            printf("%d %d", after_many(1, 2, 3, 4, 5, 6, 7, 9.0L),
                after_many(0, 2, 3, 4, 5, 6, 7, 2.0L));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("17 9");
}

TEST(Compiler, longDoubleIdentSameTranslationUnit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        long double ident_ld(long double x) { return x; }
        int main() {
            long double x;
            x = 6.0L;
            printf("%.0Lf %d", ident_ld(x), (int)ident_ld(6.0L));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6 6");
}

TEST(Compiler, staticLongDoubleLiteralInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long double x = 6.0L;
        static long double n = -6.0L;
        int main() {
            printf("%d %d", (int)x, (int)n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6 -6");
}

// 2^53+1 is exact in 80-bit and in int64, not in IEEE double.
TEST(Compiler, staticLongDoubleFromIntKeeps80BitMantissa) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long double x = 9007199254740993;
        int main() {
            printf("%.0Lf", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9007199254740993");
}

TEST(Compiler, longDoubleFromCharAndBack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char before;
        char c;
        char after;
        int main() {
            long double x;
            before = 1;
            after = 2;
            c = 6;
            x = c;
            c = 0;
            c = (char)x;
            printf("%d %.0Lf %d %d", before, x, c, after);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 6 6 2");
}

TEST(Compiler, longDoubleFromSignedCharNegative) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char c;
            long double x;
            c = -1;
            x = c;
            printf("%d", (int)x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, longDoubleToGlobalShortDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        short before;
        short g;
        short after;
        int main() {
            long double x;
            before = 1;
            after = 2;
            x = 6.0L;
            g = (short)x;
            printf("%d %d %d", before, g, after);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 6 2");
}

TEST(Compiler, longDoubleToGlobalCharDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char before;
        char g;
        char after;
        int main() {
            long double x;
            before = 1;
            after = 2;
            x = 6.0L;
            g = (char)x;
            printf("%d %d %d", before, g, after);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 6 2");
}

TEST(Compiler, longDoubleToInt128KeepsSignInHighWord) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double x;
            __int128 a;
            x = -6.0L;
            a = x;
            printf("%d %d", (int)a, a < 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-6 1");
}

TEST(Compiler, longDoubleFromDoubleAndBack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            double d;
            long double x;
            double back;
            d = 6.5;
            x = d;
            back = x;
            printf("%d %d", (int)x, (int)back);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6 6");
}

TEST(Compiler, longDoubleFromFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            float f;
            long double x;
            f = 7.25f;
            x = f;
            printf("%d", (int)x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, longDoubleCastToIntDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int before;
            long double x;
            int after;
            before = 1;
            after = 2;
            x = 6.0L;
            printf("%d %d %d", before, (int)x, after);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 6 2");
}

TEST(Compiler, longDoubleUnaryMinus) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double x;
            x = 6.0L;
            printf("%d %d", (int)-6.0L, (int)-x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-6 -6");
}

TEST(Compiler, doubleDivisionTruncatesTowardZeroOnCast) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            double n;
            n = 10.0 / 3.0;
            printf("%d %d", (int)n, (int)(0.0 - 1.9));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 -1");
}

TEST(Compiler, floatPlusFloatIsSinglePrecision) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            float a;
            float b;
            double d;
            a = 100000000.0f;
            b = 1.0f;
            d = a + b;
            printf("%d", (int)(d - 100000000.0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, floatLiteralBitsPun) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            float f;
            int *p;
            f = 1.0f;
            p = (int*)&f;
            printf("%d", *p == 0x3f800000);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, floatParameterAndReturnRoundTrip) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        float ident(float x) {
            return x;
        }
        int main() {
            float f;
            int *p;
            f = ident(1.0f);
            p = (int*)&f;
            printf("%d", *p == 0x3f800000);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, mixedFloat32AndIntParametersPackIndependently) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int combine(float f, int n) {
            return (int)f + n;
        }
        int main() {
            printf("%d", combine(3.9f, 10));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("13");
}

TEST(Compiler, floatToDoubleWidens) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            float f;
            double d;
            f = 1.5f;
            d = f;
            printf("%d", (int)(d * 2.0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, doubleToFloatNarrowsTowardZero) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            double d;
            float f;
            d = 1.9;
            f = d;
            printf("%d", (int)f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, structFloatDoesNotClobberChar) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct pair {
            char c;
            float f;
        };
        int main() {
            struct pair p;
            p.c = 7;
            p.f = 2.0f;
            printf("%d %d", (int)p.c, (int)p.f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 2");
}

TEST(Compiler, printfPromotesFloatToDouble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            float f;
            f = 3.25f;
            printf("%.2f", f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3.25");
}

TEST(Compiler, globalFloatInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        static const float k = 2.5f;
        int main() {
            printf("%d", (int)(k + k));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, unaryMinusFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            float f;
            f = 1.5f;
            printf("%d", (int)(0.0f - f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, longDoubleLiteralSizeof) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)sizeof 1.0L, (int)sizeof 1.0, (int)sizeof 1.0f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16 8 4");
}

TEST(Compiler, longDoubleLiteralOnePrints) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double x;
            x = 1.0L;
            printf("%.1Lf", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1.0");
}

TEST(Compiler, longDoubleLiteralWords) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double x;
            unsigned long *w;
            x = 1.0L;
            w = (unsigned long *)&x;
            printf("%lu %lu", w[0], w[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9223372036854775808 16383");
}

TEST(Compiler, longDoubleLiteralFortyTwoAndAHalf) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double x;
            x = 42.5L;
            printf("%.1Lf", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42.5");
}

TEST(Compiler, longDoubleAddSubMulDiv) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double a;
            long double b;
            a = 20.0L;
            b = 22.0L;
            printf("%.1Lf ", a + b);
            printf("%.1Lf ", b - a);
            printf("%.1Lf ", a * 2.0L);
            printf("%.2Lf", b / a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42.0 2.0 40.0 1.10");
}

TEST(Compiler, longDoubleUnaryMinusAndAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double a;
            a = 20.0L;
            printf("%.1Lf ", -a);
            a += 22.5L;
            printf("%.1Lf", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-20.0 42.5");
}

TEST(Compiler, longDoubleRelational) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double a;
            long double b;
            a = -1.0L;
            b = 1.0L;
            printf("%d %d %d %d ", a < b, a <= b, a > b, a >= b);
            printf("%d %d", a == a, a != b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 0 0 1 1");
}

TEST(Compiler, longDoubleConvertAndMixedAdd) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long double x;
            double d;
            int n;
            x = 42.5L;
            n = (int)x;
            d = (double)x;
            printf("%d %.1f ", n, d);
            x = (long double)20;
            printf("%.1Lf ", x + 22.5L);
            printf("%d", (int)(0.0L - 1.9L));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42 42.5 42.5 -1");
}

} // namespace
