#include "TestFixtures.h"

namespace {

TEST(Compiler, voidVariable) {
    SourceProgram program{R"prg(
        int main() {
            void a;
            return 0;
        }
    )prg"};

    program.compile();

    program.assertCompilationErrors(":3: error: variable `a` declared void");
}

TEST(Compiler, undeclaredIdentifier) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", noSuchVariable);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors(":3: error: symbol `noSuchVariable` is not defined");
}

TEST(Compiler, assignToRvalueRejected) {
    SourceProgram program{R"prg(
        int main() {
            3 = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors(":3: error: lvalue required on the left side of assignment");
}

TEST(Compiler, assignToUnaryPlusRejected) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 5;
            (+a) = 7;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors(":5: error: lvalue required on the left side of assignment");
}

TEST(Compiler, functionArityMismatchRejected) {
    SourceProgram program{R"prg(
        int f(int x) {
            return x;
        }

        int main() {
            f(1, 2);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("no match for function");
}

TEST(Compiler, functionTooFewArgumentsRejected) {
    SourceProgram program{R"prg(
        int add(int a, int b) {
            return a + b;
        }

        int main() {
            add(1);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("no match for function");
}

TEST(Compiler, unaryDereferenceOnNonPointerRejected) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            *a = 2;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("invalid type argument of ‘unary *’");
}

TEST(Compiler, postfixIncrementOnRvalueRejected) {
    SourceProgram program{R"prg(
        int main() {
            3++;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required as increment operand");
}

TEST(Compiler, prefixIncrementOnRvalueRejected) {
    SourceProgram program{R"prg(
        int main() {
            ++3;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required as increment operand");
}

TEST(Compiler, postfixDecrementOnRvalueRejected) {
    SourceProgram program{R"prg(
        int main() {
            3--;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required as increment operand");
}

TEST(Compiler, voidNamedParameterRejected) {
    SourceProgram program{R"prg(
        int f(void x) {
            return 0;
        }

        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("declared void");
}

TEST(Compiler, duplicateFunctionDefinitionRejected) {
    SourceProgram program{R"prg(
        int f() {
            return 1;
        }

        int f() {
            return 2;
        }

        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("conflicts with previous");
}

// Subscript on a non-pointer is a semantic error (arrays are not implemented as types).
TEST(Compiler, subscriptOnNonPointerRejected) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 0;
            a[0] = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("invalid type for operator[]");
}

// `int a[];` abstract array declarator is not implemented — must fail clearly.
TEST(Compiler, abstractArrayDeclaratorNotImplemented) {
    SourceProgram program{R"prg(
        int main() {
            int a[];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("not implemented");
}

// Sized arrays currently fail during AST construction (`<const_exp>` identity
// production is not registered), before semantic analysis of ArrayDeclarator.
TEST(Compiler, sizedArrayDeclaratorNotImplemented) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("not implemented");
}

} // namespace

