#include "TestFixtures.h"

namespace {

// Legal language forms: declarations, function shapes, compounds — not expression
// evaluation or pure diagnostics (those live in ExpressionTest / SemanticErrorsTest).

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

// Compound with only declarations (no statements) is a legal block.
TEST(Compiler, declarationOnlyCompound) {
    SourceProgram program{R"prg(
        int main() {
            {
                int dead;
                int alsoDead;
            }
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Grammar allows a declaration of only type specs and no declarators (`int;`).
// Contract: accepted; introduces no name and does not disturb following code.
// Exercises AST builder path for `<decl> ::= <decl_specs> ;`.
TEST(Compiler, specifierOnlyDeclarationAccepted) {
    SourceProgram program{R"prg(
        int main() {
            int;
            int a;
            a = 1;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// `float` is a recognized type specifier (full float arithmetic is not required).
// Contract: the front end accepts float object declarations and still produces a working program.
TEST(Compiler, floatTypeSpecifierAccepted) {
    SourceProgram program{R"prg(
        int main() {
            float f;
            int a;
            a = 1;
            f = a;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Function returning pointer via declarator indirection; result used as call arg.
TEST(Compiler, functionReturningPointer) {
    SourceProgram program{R"prg(
        int *addr(int *p) {
            return p;
        }

        void set(int *p, int v) {
            *p = v;
            return;
        }

        int main() {
            int a;
            a = 0;
            set(addr(&a), 42);
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Multiple pointer declarators in one declaration.
TEST(Compiler, multiPointerDeclarators) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            int *p, *q;
            a = 1;
            b = 2;
            p = &a;
            q = &b;
            printf("%d %d", *p, *q);
            *p = 3;
            *q = 4;
            printf(" %d %d", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}

} // namespace
