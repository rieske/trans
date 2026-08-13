#include "TestFixtures.h"

namespace {

// GNU __real__ / __imag__ unary operators (fuzz regressions and surface gaps).

TEST(Compiler, gnuRealImagOperators) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex float z;
            z = 3.0f;
            printf("%d %d", (int)__real__(z), (int)__imag__(z));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 0");
}

TEST(Compiler, gnuRealImagUnparenthesizedAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex double z;
            __real__ z = 5.0;
            __imag__ z = 2.0;
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 2");
}

TEST(Compiler, gnuRealOfRvalueCallIsNotAssignable) {
    SourceProgram program{R"prg(
        _Complex double id(_Complex double z) { return z; }
        int main() {
            _Complex double z;
            z = 1.0;
            __real__(id(z)) = 2.0;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required");
}

TEST(Compiler, gnuImagOfRvalueCallIsNotAssignable) {
    SourceProgram program{R"prg(
        _Complex double id(_Complex double z) { return z; }
        int main() {
            _Complex double z;
            z = 1.0;
            __imag__(id(z)) = 2.0;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required");
}

TEST(Compiler, gnuRealOfCallStillReads) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        _Complex double id(_Complex double z) { return z; }
        int main() {
            _Complex double z;
            __real__ z = 7.0;
            __imag__ z = 1.0;
            printf("%d", (int)__real__(id(z)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, gnuRealOfFloatIsAssignable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            float x;
            x = 1.0f;
            __real__ x = 4.0f;
            printf("%d %d", (int)__real__ x, (int)__imag__ x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 0");
}

TEST(Compiler, gnuImagOfFloatIsNotAssignable) {
    SourceProgram program{R"prg(
        int main() {
            float x;
            x = 1.0f;
            __imag__ x = 2.0f;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required");
}

TEST(Compiler, gnuRealOfConstantIsNotAssignable) {
    SourceProgram program{R"prg(
        int main() {
            __real__(1.0) = 2.0;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required");
}

TEST(Compiler, gnuImagOfConstantIsNotAssignable) {
    SourceProgram program{R"prg(
        int main() {
            __imag__(1.0) = 2.0;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required");
}

TEST(Compiler, gnuRealOfSizeofInCastOperand) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", (int)__real__(double)sizeof(double));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, gnuRealOfCastOperand) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            double x;
            x = 3.0;
            printf("%d", (int)__real__(double)x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, gnuRealOfCompoundLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", (int)__real__(_Complex double){ 4.0 });
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, gnuRealOfBinaryKeepsPrecedence) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex double z;
            __real__ z = 3.0;
            __imag__ z = 0.0;
            printf("%d", (int)(__real__ z + 2.0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, gnuNestedRealImag) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex double z;
            __real__ z = 6.0;
            __imag__ z = 0.0;
            printf("%d", (int)(__real__ __real__ z));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, gnuRealImagCompoundAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex double z;
            __real__ z = 3.0;
            __imag__ z = 1.0;
            __real__ z += 2.0;
            __imag__ z *= 4.0;
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 4");
}

TEST(Compiler, addressOfGnuRealIsAssignableThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex double z;
            double *p;
            __real__ z = 5.0;
            __imag__ z = 2.0;
            p = &__real__ z;
            *p = 7.0;
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 2");
}

TEST(Compiler, gnuRealImagOfArrayElementAndPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex double a[2];
            _Complex double *p;
            __real__(a[0]) = 1.0;
            __imag__(a[0]) = 2.0;
            __real__(a[1]) = 3.0;
            __imag__(a[1]) = 4.0;
            p = &a[1];
            __real__(*p) = 5.0;
            printf("%d %d %d %d",
                    (int)__real__(a[0]), (int)__imag__(a[0]),
                    (int)__real__(a[1]), (int)__imag__(a[1]));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 5 4");
}

TEST(Compiler, gnuRealOfStatementExpression) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex double z;
            __real__ z = 9.0;
            __imag__ z = 1.0;
            printf("%d", (int)__real__({ z; }));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, gnuRealImagLongDoubleParts) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex long double z;
            __real__ z = 8.0L;
            __imag__ z = 9.0L;
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 9");
}

TEST(Compiler, gnuRealOnIntegerIsError) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 1;
            return (int)__real__ n;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("__real__/__imag__ requires a real or complex type");
}

// Imag part of an rvalue is readable (copyPart path), but not assignable.
TEST(Compiler, gnuImagOfCallStillReads) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        _Complex double id(_Complex double z) { return z; }
        int main() {
            _Complex double z;
            __real__ z = 3.0;
            __imag__ z = 4.0;
            printf("%d", (int)__imag__(id(z)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

// Member access is a valid cast-expression under __real__/__imag__.
TEST(Compiler, gnuRealImagOfStructMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { _Complex double z; };
        int main() {
            struct S s;
            __real__ s.z = 2.0;
            __imag__ s.z = 5.0;
            printf("%d %d", (int)__real__ s.z, (int)__imag__ s.z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 5");
}

} // namespace
