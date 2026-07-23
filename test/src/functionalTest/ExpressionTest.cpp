#include "TestFixtures.h"

namespace {

TEST(Compiler, unaryPlusPreserves) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d", +a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0", "0");
    program.runAndExpect("5", "5");
    program.runAndExpect("-2", "-2");
}

TEST(Compiler, chainedAssignment) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            int c;
            c = 3;
            a = b = c;
            printf("%d %d %d", a, b, c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 3 3");
}

TEST(Compiler, commaOperatorDiscardsLeft) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            b = 1;
            a = (b = 2, 3);
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 2");
}

TEST(Compiler, logicalAndSkipsRight) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            0 && ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, logicalOrSkipsRight) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            1 || ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

// Default return type on a function definition (historical C: missing type → int).
TEST(Compiler, defaultReturnTypeFunctionDefinition) {
    SourceProgram program{R"prg(
        main() {
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, defaultReturnTypeHelperFunction) {
    SourceProgram program{R"prg(
        add(int a, int b) {
            return a + b;
        }

        int main() {
            printf("%d", add(20, 22));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Float locals are accepted by the type system (no float arithmetic required).
TEST(Compiler, floatLocalDeclaration) {
    SourceProgram program{R"prg(
        int main() {
            float f;
            int a;
            a = 1;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Specifier-only declaration (no declarators) is a no-op at block scope.
TEST(Compiler, specifierOnlyDeclaration) {
    SourceProgram program{R"prg(
        int main() {
            int;
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Multiple declarators in one declaration, with and without initializers.
TEST(Compiler, multiDeclaratorWithInitializers) {
    SourceProgram program{R"prg(
        int main() {
            int a = 2, b = 3, c;
            c = a + b;
            printf("%d %d %d", a, b, c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 3 5");
}

TEST(Compiler, multiGlobalDeclaratorsWithInitializers) {
    SourceProgram program{R"prg(
        int a = 10, b = 32;

        int main() {
            printf("%d", a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// `(void)` is an empty parameter list (not a void-typed argument).
TEST(Compiler, voidParameterListMeansNoArgs) {
    SourceProgram program{R"prg(
        int f(void) {
            return 7;
        }

        int main() {
            printf("%d", f());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

// Floating constants are recognized by the scanner but not lowered to AST constants.
TEST(Compiler, floatingConstantNotImplemented) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1.5;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("not implemented");
}

TEST(Compiler, logicalAndEvaluatesRight) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            1 && ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, logicalOrEvaluatesRight) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int* p;
            n = 0;
            p = &n;
            0 || ++(*p);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, doubleNotOnComparison) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            scanf("%ld %ld", &a, &b);
            printf("%d", !!(a == b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0", "1");
    program.runAndExpect("1 0", "0");
}

TEST(Compiler, pointerEquality) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            a = 1;
            b = 2;
            printf("%d %d", &a == &a, &a == &b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

} // namespace
