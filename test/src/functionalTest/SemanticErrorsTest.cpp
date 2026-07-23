#include "TestFixtures.h"

#include <string>

namespace {

// Catalog of compile-time rejection contracts. One row = one diagnostic pin.
// Prefer full product message fragments over generic "error" / "not implemented".
struct SemanticErrorCase {
    const char *name;
    const char *source;
    const char *errorFragment;
};

class SemanticErrorCatalog : public testing::TestWithParam<SemanticErrorCase> {};

TEST_P(SemanticErrorCatalog, RejectsWithMessage) {
    const SemanticErrorCase &c = GetParam();
    SourceProgram program{c.source};
    program.compile();
    program.assertCompilationErrors(c.errorFragment);
}

INSTANTIATE_TEST_SUITE_P(Compiler, SemanticErrorCatalog,
                         testing::Values(
                             SemanticErrorCase{
                                 "voidVariable",
                                 R"prg(
        int main() {
            void a;
            return 0;
        }
    )prg",
                                 ":3: error: variable `a` declared void",
                             },
                             SemanticErrorCase{
                                 "undeclaredIdentifier",
                                 R"prg(
        int main() {
            printf("%d", noSuchVariable);
            return 0;
        }
    )prg",
                                 ":3: error: symbol `noSuchVariable` is not defined",
                             },
                             SemanticErrorCase{
                                 "assignToRvalue",
                                 R"prg(
        int main() {
            3 = 1;
            return 0;
        }
    )prg",
                                 ":3: error: lvalue required on the left side of assignment",
                             },
                             SemanticErrorCase{
                                 "assignToUnaryPlus",
                                 R"prg(
        int main() {
            int a;
            a = 5;
            (+a) = 7;
            return 0;
        }
    )prg",
                                 ":5: error: lvalue required on the left side of assignment",
                             },
                             SemanticErrorCase{
                                 "arityTooManyArgs",
                                 R"prg(
        int f(int x) {
            return x;
        }

        int main() {
            f(1, 2);
            return 0;
        }
    )prg",
                                 "no match for function",
                             },
                             SemanticErrorCase{
                                 "arityTooFewArgs",
                                 R"prg(
        int add(int a, int b) {
            return a + b;
        }

        int main() {
            add(1);
            return 0;
        }
    )prg",
                                 "no match for function",
                             },
                             SemanticErrorCase{
                                 "unaryDerefNonPointer",
                                 R"prg(
        int main() {
            int a;
            a = 1;
            *a = 2;
            return 0;
        }
    )prg",
                                 "invalid type argument of ‘unary *’",
                             },
                             SemanticErrorCase{
                                 "postfixIncrementRvalue",
                                 R"prg(
        int main() {
            3++;
            return 0;
        }
    )prg",
                                 "lvalue required as increment operand",
                             },
                             SemanticErrorCase{
                                 "prefixIncrementRvalue",
                                 R"prg(
        int main() {
            ++3;
            return 0;
        }
    )prg",
                                 "lvalue required as increment operand",
                             },
                             SemanticErrorCase{
                                 "postfixDecrementRvalue",
                                 R"prg(
        int main() {
            3--;
            return 0;
        }
    )prg",
                                 "lvalue required as increment operand",
                             },
                             SemanticErrorCase{
                                 "voidNamedParameter",
                                 R"prg(
        int f(void x) {
            return 0;
        }

        int main() {
            return 0;
        }
    )prg",
                                 "function argument ‘x’ declared void",
                             },
                             SemanticErrorCase{
                                 "duplicateFunctionDefinition",
                                 R"prg(
        int f() {
            return 1;
        }

        int f() {
            return 2;
        }

        int main() {
            return 0;
        }
    )prg",
                                 "conflicts with previous",
                             },
                             SemanticErrorCase{
                                 "subscriptOnNonPointer",
                                 R"prg(
        int main() {
            int a;
            a = 0;
            a[0] = 1;
            return 0;
        }
    )prg",
                                 "invalid type for operator[]",
                             },
                             SemanticErrorCase{
                                 "abstractArrayDeclarator",
                                 R"prg(
        int main() {
            int a[];
            return 0;
        }
    )prg",
                                 "abstract array declarator is not implemented yet",
                             },
                             // Sized arrays currently fail earlier: <const_exp> identity is unregistered.
                             SemanticErrorCase{
                                 "sizedArrayBlockedOnConstExpIdentity",
                                 R"prg(
        int main() {
            int a[3];
            return 0;
        }
    )prg",
                                 "language construct not implemented yet (production `<const_exp> ::= <conditional_exp>`)",
                             },
                             SemanticErrorCase{
                                 "floatingConstant",
                                 R"prg(
        int main() {
            int a;
            a = 1.5;
            return 0;
        }
    )prg",
                                 "floating constants not implemented yet",
                             },
                             SemanticErrorCase{
                                 "breakOutsideLoop",
                                 R"prg(
        int main() {
            break;
            return 0;
        }
    )prg",
                                 "not in loop",
                             },
                             SemanticErrorCase{
                                 "continueOutsideLoop",
                                 R"prg(
        int main() {
            continue;
            return 0;
        }
    )prg",
                                 "not in loop",
                             },
                             SemanticErrorCase{
                                 "globalCollidesWithFunction",
                                 R"prg(
        int foo;

        int foo() {
            return 0;
        }

        int main() {
            return 0;
        }
    )prg",
                                 "conflicts with global variable of the same name",
                             },
                             SemanticErrorCase{
                                 "declarationCollidesWithFunction",
                                 R"prg(
        int foo() {
            return 1;
        }

        int foo;

        int main() {
            return 0;
        }
    )prg",
                                 "declaration conflicts with function of the same name",
                             },
                             SemanticErrorCase{
                                 "nonConstantGlobalInitializer",
                                 R"prg(
        int f() {
            return 1;
        }

        int g = f();

        int main() {
            return 0;
        }
    )prg",
                                 "global initializer is not a constant expression",
                             },
                             // Unknown char escape fails constant fold → same global-init rule as non-const exprs.
                             SemanticErrorCase{
                                 "invalidCharEscapeNotConstant",
                                 R"prg(
        int g = '\q';

        int main() {
            return 0;
        }
    )prg",
                                 "global initializer is not a constant expression",
                             },
                             // Unary ~ is not folded by evaluateConstant (only + - !).
                             SemanticErrorCase{
                                 "bitwiseNotGlobalNotConstant",
                                 R"prg(
        int g = ~5;

        int main() {
            return 0;
        }
    )prg",
                                 "global initializer is not a constant expression",
                             },
                             // Pointer subscript type-checks, then backend rejects array access codegen.
                             SemanticErrorCase{
                                 "pointerSubscriptCodegenNotImplemented",
                                 R"prg(
        int main() {
            int a;
            int *p;
            a = 1;
            p = &a;
            printf("%d", p[0]);
            return 0;
        }
    )prg",
                                 "code generation for array access is not implemented",
                             },
                             SemanticErrorCase{
                                 "braceInitializerNotImplemented",
                                 R"prg(
        int main() {
            int a = {1};
            return 0;
        }
    )prg",
                                 "language construct not implemented yet (production `<initializer_list> ::= <initializer>`)",
                             }),
                         [](const testing::TestParamInfo<SemanticErrorCase> &info) { return std::string{info.param.name}; });

} // namespace
