#include "TestFixtures.h"

namespace {

TEST(Compiler, complexSizeofAndAlignofLayout) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Cf { char c; _Complex float z; };
        struct Cd { char c; _Complex double z; };
        struct Cld { char c; _Complex long double z; };
        int main() {
            printf("%d %d %d %d %d %d",
                    (int)sizeof(_Complex float),
                    (int)sizeof(_Complex double),
                    (int)sizeof(_Complex long double),
                    (int)sizeof(struct Cf),
                    (int)sizeof(struct Cd),
                    (int)sizeof(struct Cld));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 16 32 12 24 48");
}

TEST(Compiler, complexSpecifierOrder) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex float a;
            float _Complex b;
            _Complex double c;
            double _Complex d;
            _Complex long double e;
            long double _Complex f;
            printf("%d %d %d %d %d %d",
                    (int)sizeof(a), (int)sizeof(b),
                    (int)sizeof(c), (int)sizeof(d),
                    (int)sizeof(e), (int)sizeof(f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 8 16 16 32 32");
}

TEST(Compiler, complexTypedefAndPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef _Complex float Cf;
        int main() {
            Cf z;
            Cf *p;
            p = &z;
            printf("%d %d", (int)sizeof(z), (int)sizeof(p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 8");
}

TEST(Compiler, complexAssignAddSubNegFromReal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex float a;
            _Complex float b;
            float f;
            int n;
            a = 3.0f;
            b = 4.0f;
            a = a + b;
            n = (int)a;
            a = 10.0f;
            a = a - 3.0f;
            a = -a;
            f = a;
            printf("%d %d", n, (int)f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 -7");
}

TEST(Compiler, complexMixedRealUacAndWiden) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex float z;
            _Complex double d;
            z = 1.0f;
            z = z + 2.0;
            d = z;
            d = d + 3;
            printf("%d %d", (int)z, (int)d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 6");
}

TEST(Compiler, complexLongDoubleAddFromReal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex long double z;
            z = 20.0L;
            z = z + 22.5L;
            printf("%d", (int)z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, complexMulDivFromReal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex float a;
            _Complex float b;
            a = 3.0f;
            b = 4.0f;
            a = a * b;
            b = 8.0f;
            b = b / 2.0f;
            printf("%d %d", (int)a, (int)b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 4");
}

TEST(Compiler, complexEquality) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex float a;
            _Complex float b;
            a = 1.0f;
            b = 1.0f;
            printf("%d ", a == b);
            a = 2.0f;
            printf("%d ", a != b);
            printf("%d", a == b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 0");
}

TEST(Compiler, complexCompoundAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex double z;
            z = 5.0;
            z += 2.0;
            z -= 1.0;
            printf("%d", (int)z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, complexPointerIncrementScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex float a[2];
            _Complex float *p;
            a[0] = 1.0f;
            a[1] = 2.0f;
            p = &a[0];
            ++p;
            printf("%d", (int)*p);
            p--;
            printf(" %d", (int)*p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

TEST(Compiler, complexIdentityPassReturn) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        _Complex float ident_cf(_Complex float x) {
            return x;
        }
        _Complex double ident_cd(_Complex double x) {
            return x;
        }
        _Complex long double ident_cld(_Complex long double x) {
            return x;
        }
        int main() {
            _Complex float a;
            _Complex double b;
            _Complex long double c;
            a = ident_cf(a);
            b = ident_cd(b);
            c = ident_cld(c);
            printf("%d %d %d", (int)sizeof(a), (int)sizeof(b), (int)sizeof(c));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 16 32");
}

} // namespace
