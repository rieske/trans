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

INSTANTIATE_TEST_SUITE_P(Compiler, SemanticErrorCatalog, testing::Values(
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
            int printf(const char *, ...);
            int scanf(const char *, ...);
            int main() {
                printf("%d", noSuchVariable);
                return 0;
            }
        )prg",
        "symbol `noSuchVariable` is not defined",
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
        "variadicTooFewNamedArgs",
        R"prg(
            int first(int n, ...) {
                return n;
            }

            int main() {
                first();
                return 0;
            }
        )prg",
        "no match for function",
    },
    SemanticErrorCase{
        "prototypeVariadicMismatch",
        R"prg(
            int first(int n);
            int first(int n, ...) {
                return n;
            }

            int main() {
                return first(1);
            }
        )prg",
        "definition conflicts with previous",
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
        "pointerPlusPointer",
        R"prg(
            int main() {
                int a;
                int *p;
                int *q;
                p = &a;
                q = &a;
                p = p + q;
                return 0;
            }
        )prg",
        "invalid operands to pointer arithmetic",
    },
    SemanticErrorCase{
        "incompleteLocalArray",
        R"prg(
            int main() {
                int a[];
                return 0;
            }
        )prg",
        "incomplete type",
    },
    SemanticErrorCase{
        "incompleteFileScopeArray",
        R"prg(
            int a[];
            int main() {
                return 0;
            }
        )prg",
        "incomplete type",
    },
    SemanticErrorCase{
        "genericNoMatch",
        R"prg(
            int main() {
                return _Generic(0, char: 1);
            }
        )prg",
        "no matching association",
    },
    SemanticErrorCase{
        "genericDuplicateDefault",
        R"prg(
            int main() {
                return _Generic(0, default: 1, default: 2);
            }
        )prg",
        "duplicate default",
    },
    SemanticErrorCase{
        "genericMultipleMatch",
        R"prg(
            int main() {
                return _Generic(0, int: 1, int: 2);
            }
        )prg",
        "multiple matching associations",
    },
    SemanticErrorCase{
        "genericTypeofAssociationUnknown",
        R"prg(
            int main() {
                return _Generic(0, typeof(nope): 1);
            }
        )prg",
        "cannot determine type of typeof operand",
    },
    SemanticErrorCase{
        "genericMatchingArmUndeclared",
        R"prg(
            int main() {
                return _Generic(0, int: nope);
            }
        )prg",
        "symbol `nope` is not defined",
    },
    SemanticErrorCase{
        "genericUndeclaredControlling",
        R"prg(
            int main() {
                return _Generic(nope, int: 1, default: 2);
            }
        )prg",
        "symbol `nope` is not defined",
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
    // ~ folds for constants; non-constant operand still fails global init.
    SemanticErrorCase{
        "bitwiseNotGlobalNotConstant",
        R"prg(
            int x;
            int g = ~x;

            int main() {
                return 0;
            }
        )prg",
        "global initializer is not a constant expression",
    },
    SemanticErrorCase{
        "staticFunctionAfterNonStaticPrototype",
        R"prg(
            int f(void);
            static int f(void) {
                return 1;
            }
            int main(void) {
                return f();
            }
        )prg",
        "static declaration of `f` follows non-static",
    },
    SemanticErrorCase{
        "staticFunctionPrototypeAfterNonStatic",
        R"prg(
            int f(void);
            static int f(void);
            int main(void) {
                return 0;
            }
        )prg",
        "static declaration of `f` follows non-static",
    },
    SemanticErrorCase{
        "staticObjectAfterExtern",
        R"prg(
            extern int x;
            static int x;
            int main(void) {
                return 0;
            }
        )prg",
        "static declaration of `x` follows non-static",
    },
    SemanticErrorCase{
        "staticObjectAfterTentative",
        R"prg(
            int x;
            static int x;
            int main(void) {
                return 0;
            }
        )prg",
        "static declaration of `x` follows non-static",
    },
    SemanticErrorCase{
        "nonStaticObjectAfterStatic",
        R"prg(
            static int x;
            int x;
            int main(void) {
                return 0;
            }
        )prg",
        "non-static declaration of `x` follows static",
    },
    SemanticErrorCase{
        "twoObjectInitializers",
        R"prg(
            int x = 1;
            int x = 2;
            int main(void) {
                return 0;
            }
        )prg",
        "declaration conflicts",
    },
    SemanticErrorCase{
        "twoObjectInitializersAfterNonConstant",
        R"prg(
            int f(void);
            int x = f();
            int x = 2;
            int main(void) {
                return 0;
            }
        )prg",
        "declaration conflicts",
    },
    SemanticErrorCase{
        "objectTypeMismatchOnRedecl",
        R"prg(
            extern int x;
            extern long x;
            int main(void) {
                return 0;
            }
        )prg",
        "declaration conflicts",
    },
    SemanticErrorCase{
        "objectQualifierMismatchOnRedecl",
        R"prg(
            extern int x;
            extern const int x;
            int main(void) {
                return 0;
            }
        )prg",
        "declaration conflicts",
    },
    SemanticErrorCase{
        "arrayBoundMismatchOnRedecl",
        R"prg(
            extern char a[2];
            char a[1];
            int main(void) {
                return 0;
            }
        )prg",
        "declaration conflicts",
    },
    SemanticErrorCase{
        "multidimInnerBoundMismatchOnRedecl",
        R"prg(
            extern int a[][3];
            int a[2][4];
            int main(void) {
                return 0;
            }
        )prg",
        "declaration conflicts",
    },
    SemanticErrorCase{
        "incompleteArrayQualifierMismatchOnRedecl",
        R"prg(
            extern char a[];
            const char a[1];
            int main(void) {
                return 0;
            }
        )prg",
        "declaration conflicts",
    },
    SemanticErrorCase{
        "complexRelationalLt",
        R"prg(
            int main() {
                _Complex float a;
                _Complex float b;
                return a < b;
            }
        )prg",
        "invalid operands to relational",
    },
    SemanticErrorCase{
        "complexRelationalGe",
        R"prg(
            int main() {
                _Complex double a;
                return a >= 1.0;
            }
        )prg",
        "invalid operands to relational",
    },
    SemanticErrorCase{
        "complexRemainder",
        R"prg(
            int main() {
                _Complex float a;
                a = 1.0f;
                return (int)(a % a);
            }
        )prg",
        "invalid operands to %",
    },
    SemanticErrorCase{
        "complexBitwiseAnd",
        R"prg(
            int main() {
                _Complex float a;
                a = 1.0f;
                return (int)(a & a);
            }
        )prg",
        "invalid operands to bitwise",
    },
    SemanticErrorCase{
        "complexShiftLeft",
        R"prg(
            int main() {
                _Complex float a;
                a = 1.0f;
                return (int)(a << 1);
            }
        )prg",
        "argument of type int required for shift",
    },
    SemanticErrorCase{
        "complexShiftCount",
        R"prg(
            int main() {
                _Complex float a;
                a = 1.0f;
                return 1 << a;
            }
        )prg",
        "argument of type int required for shift",
    },
    SemanticErrorCase{
        "complexPrefixIncrement",
        R"prg(
            int main() {
                _Complex float a;
                ++a;
                return 0;
            }
        )prg",
        "invalid operand to increment",
    },
    SemanticErrorCase{
        "complexPrefixDecrement",
        R"prg(
            int main() {
                _Complex double a;
                --a;
                return 0;
            }
        )prg",
        "invalid operand to increment",
    },
    SemanticErrorCase{
        "complexPostfixIncrement",
        R"prg(
            int main() {
                _Complex float a;
                a++;
                return 0;
            }
        )prg",
        "invalid operand to increment",
    },
    SemanticErrorCase{
        "complexPostfixDecrement",
        R"prg(
            int main() {
                _Complex long double a;
                a--;
                return 0;
            }
        )prg",
        "invalid operand to increment",
    },
    SemanticErrorCase{
        "structPrefixIncrement",
        R"prg(
            struct S { int x; };
            int main() {
                struct S s;
                ++s;
                return 0;
            }
        )prg",
        "invalid operand to increment",
    },
    SemanticErrorCase{
        "unionPostfixDecrement",
        R"prg(
            union U { int x; };
            int main() {
                union U u;
                u--;
                return 0;
            }
        )prg",
        "invalid operand to increment",
    },
    SemanticErrorCase{
        "arrayPrefixIncrement",
        R"prg(
            int main() {
                int a[2];
                ++a;
                return 0;
            }
        )prg",
        "invalid operand to increment",
    },
    SemanticErrorCase{
        "bitFieldAddressOf",
        R"prg(
            struct S { int x:3; };
            int main() {
                struct S s;
                int *p;
                p = &s.x;
                return 0;
            }
        )prg",
        "cannot take address of bit-field",
    },
    SemanticErrorCase{
        "bitFieldSizeof",
        R"prg(
            struct S { int x:3; };
            int main() {
                struct S s;
                return sizeof s.x;
            }
        )prg",
        "sizeof",
    },
    SemanticErrorCase{
        "bitFieldWidthTooWide",
        R"prg(
            struct S { int x:33; };
            int main() {
                return 0;
            }
        )prg",
        "width of bit-field",
    },
    SemanticErrorCase{
        "bitFieldNamedZeroWidth",
        R"prg(
            struct S { int x:0; };
            int main() {
                return 0;
            }
        )prg",
        "zero width for bit-field",
    },
    SemanticErrorCase{
        "bitFieldNonIntegerType",
        R"prg(
            struct S { float f:3; };
            int main() {
                return 0;
            }
        )prg",
        "bit-field",
    },
    SemanticErrorCase{
        "bitFieldInt128Type",
        R"prg(
            struct S { __int128 x:8; };
            int main() {
                return 0;
            }
        )prg",
        "bit-field type is too wide",
    },
    SemanticErrorCase{
        "sizeofIncompleteArrayBeforeCompletion",
        R"prg(
            extern char a[];
            int f(void) {
                return sizeof a;
            }
            char a[1];
            int main(void) {
                return f();
            }
        )prg",
        "incomplete type",
    }
), [](const testing::TestParamInfo<SemanticErrorCase> &info) { return std::string{info.param.name}; });

} // namespace
