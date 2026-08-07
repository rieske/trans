#include "TestFixtures.h"

namespace {

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

TEST(Compiler, printfDoubleDoesNotDisplaceFollowingInt) {
    SourceProgram program{R"prg(
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
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, floatConstantUsedInDoubleArithmetic) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

TEST(Compiler, doubleDivisionTruncatesTowardZeroOnCast) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

} // namespace
