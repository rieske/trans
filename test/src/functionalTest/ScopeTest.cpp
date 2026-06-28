#include "TestFixtures.h"

namespace {

TEST(Compiler, multipleDeclaratorsInOneDecl) {
    SourceProgram program{R"prg(
        int main() {
            int a, b, c;
            a = 1;
            b = 2;
            c = 3;
            printf("%d %d %d", a, b, c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, initializedLocals) {
    SourceProgram program{R"prg(
        int main() {
            int a = 4;
            int b = 5;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 5");
}

TEST(Compiler, innerBlockShadowsOuter) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            {
                int a;
                a = 2;
                printf("%d", a);
            }
            printf(" %d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

TEST(Compiler, innerBlockSeparateVariable) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            {
                int b;
                b = 2;
                printf("%d", b);
            }
            printf(" %d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

// Sibling blocks each introduce their own `a`; must not clash with each other or outer `a`.
TEST(Compiler, siblingBlocksShadowIndependently) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            {
                int a;
                a = 2;
                printf("%d", a);
            }
            {
                int a;
                a = 3;
                printf(" %d", a);
            }
            printf(" %d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 3 1");
}

// C: parameters and the function's outermost block share one scope - cannot redeclare a parameter.
TEST(Compiler, parameterRedeclaredInOutermostBlockRejected) {
    SourceProgram program{R"prg(
        int f(int a) {
            int a;
            a = 2;
            return a;
        }

        int main() {
            printf("%d", f(1));
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("declaration conflicts");
}

// Nested block may shadow a parameter (distinct inner scope).
TEST(Compiler, parameterShadowedInNestedBlock) {
    SourceProgram program{R"prg(
        int f(int a) {
            {
                int a;
                a = 2;
                printf("%d", a);
            }
            printf(" %d", a);
            return 0;
        }

        int main() {
            f(1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

// Using a parameter in the outermost body (same scope as the parameter) must work.
TEST(Compiler, parameterUsedInOutermostBlock) {
    SourceProgram program{R"prg(
        int f(int a) {
            printf("%d", a);
            return a;
        }

        int main() {
            printf(" %d", f(7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 7");
}

TEST(Compiler, charPromotedInArithmetic) {
    SourceProgram program{R"prg(
        int main() {
            char c;
            c = 2;
            printf("%d", c + 3);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

} // namespace
