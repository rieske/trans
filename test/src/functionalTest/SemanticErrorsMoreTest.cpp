#include "SemanticErrorCatalog.h"

namespace {

INSTANTIATE_TEST_SUITE_P(CompilerMore, SemanticErrorCatalog, testing::Values(
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
        "invalid operands to binary bitwise",
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
    },
    SemanticErrorCase{
        "caseOutsideSwitch",
        R"prg(
            int main() {
                case 1:
                return 0;
            }
        )prg",
        "case label not within a switch statement",
    },
    SemanticErrorCase{
        "gotoUndefinedLabel",
        R"prg(
            int main() {
                goto missing;
                return 0;
            }
        )prg",
        "label `missing` used but not defined",
    },
    SemanticErrorCase{
        "memberOfNonStruct",
        R"prg(
            int main() {
                int x;
                x.y = 1;
                return 0;
            }
        )prg",
        "request for member in non-struct",
    },
    SemanticErrorCase{
        "unknownStructMember",
        R"prg(
            struct S { int a; };
            int main() {
                struct S s;
                s.nope = 1;
                return 0;
            }
        )prg",
        "no member named `nope` in structure or union",
    },
    SemanticErrorCase{
        "negativeArraySize",
        R"prg(
            int main() {
                int a[-1];
                return 0;
            }
        )prg",
        "array size is negative",
    },
    SemanticErrorCase{
        "nonConstantLocalArraySize",
        R"prg(
            int main() {
                int n;
                n = 3;
                int a[n];
                return 0;
            }
        )prg",
        "array size is not a non-negative constant expression",
    },
    SemanticErrorCase{
        "nonConstantIncompleteArrayDesignator",
        R"prg(
            int main() {
                int n;
                n = 2;
                int a[] = { [n] = 1 };
                return 0;
            }
        )prg",
        "designated array index is not a constant expression",
    },
    SemanticErrorCase{
        "duplicateLabel",
        R"prg(
            int main() {
                again:
                again:
                return 0;
            }
        )prg",
        "duplicate label `again`",
    },
    SemanticErrorCase{
        "calledObjectNotFunction",
        R"prg(
            int main() {
                int x;
                x = 1;
                x();
                return 0;
            }
        )prg",
        "called object `x` is not a function",
    },
    SemanticErrorCase{
        "localSymbolRedeclaration",
        R"prg(
            int main() {
                int x;
                int x;
                x = 1;
                return 0;
            }
        )prg",
        "conflicts with previous declaration",
    },
    SemanticErrorCase{
        "parameterConflictsWithStaticLocal",
        R"prg(
            int f(int n) {
                static int n;
                return n;
            }
            int main(void) {
                return f(0);
            }
        )prg",
        "conflicts with previous",
    },
    SemanticErrorCase{
        "parameterConflictsWithBlockExtern",
        R"prg(
            int f(int n) {
                extern int n;
                return n;
            }
            int main(void) {
                return f(0);
            }
        )prg",
        "conflicts with previous",
    },
    SemanticErrorCase{
        "blockExternTypeMismatchWithFileScope",
        R"prg(
            char x;
            int f(void) {
                extern int x;
                return x;
            }
            int main(void) {
                return f();
            }
        )prg",
        "conflicts with previous",
    },
    SemanticErrorCase{
        "multipleDefaultLabelsInSwitch",
        R"prg(
            int main() {
                int x;
                x = 1;
                switch (x) {
                    default:
                        break;
                    default:
                        break;
                }
                return 0;
            }
        )prg",
        "multiple default labels in switch",
    },
    SemanticErrorCase{
        "caseLabelNotConstantExpression",
        R"prg(
            int main() {
                int x;
                int y;
                x = 1;
                y = 2;
                switch (x) {
                    case y:
                        break;
                }
                return 0;
            }
        )prg",
        "case label is not a constant expression",
    },
    SemanticErrorCase{
        "wrongNumberOfArgumentsToVaStart",
        R"prg(
            int f(int n, ...) {
                __builtin_va_list ap;
                __builtin_va_start();
                __builtin_va_end(ap);
                return n;
            }
            int main() {
                return f(1, 2);
            }
        )prg",
        "wrong number of arguments to",
    },
    SemanticErrorCase{
        "wrongNumberOfArgumentsToVaCopy",
        R"prg(
            int f(int n, ...) {
                __builtin_va_list ap;
                __builtin_va_list cp;
                __builtin_va_start(ap, n);
                __builtin_va_copy(cp);
                __builtin_va_end(ap);
                return n;
            }
            int main() {
                return f(1, 2);
            }
        )prg",
        "wrong number of arguments to __builtin_va_copy",
    },
    SemanticErrorCase{
        "baseOfArrowIsNotPointer",
        R"prg(
            int printf(const char *, ...);
            struct S { int a; };
            int main() {
                struct S s;
                s.a = 1;
                printf("%d", s->a);
                return 0;
            }
        )prg",
        "base of '->' is not a pointer",
    },
    SemanticErrorCase{
        "assignStructToScalar",
        R"prg(
            struct S { int a; };
            int main() {
                struct S s;
                int x;
                s.a = 1;
                x = s;
                return 0;
            }
        )prg",
        "type mismatch",
    },
    SemanticErrorCase{
        "structInitializerToScalar",
        R"prg(
            struct S { int a; };
            int main() {
                struct S s;
                s.a = 1;
                int x = s;
                return 0;
            }
        )prg",
        "type mismatch",
    },
    SemanticErrorCase{
        "scalarInitializerToStruct",
        R"prg(
            struct S { int a; };
            int main() {
                struct S s = 1;
                return 0;
            }
        )prg",
        "type mismatch",
    },
    SemanticErrorCase{
        "multipleStorageClasses",
        R"prg(
            int main() {
                static auto int x;
                x = 1;
                return 0;
            }
        )prg",
        "multiple storage classes",
    },
    SemanticErrorCase{
        "sizeofUndeclared",
        R"prg(
            int printf(const char *, ...);
            int main() {
                printf("%lu", (unsigned long)sizeof(nope));
                return 0;
            }
        )prg",
        "is not defined",
    },
    SemanticErrorCase{
        "stringLiteralOnInt",
        R"prg(
            int g = "hi";
            int main() {
                return 0;
            }
        )prg",
        "string literal initializer",
    },
    SemanticErrorCase{
        "localStringLiteralOnInt",
        R"prg(
            int main() {
                int x = "hi";
                return x;
            }
        )prg",
        "string literal initializer",
    },
    SemanticErrorCase{
        "prototypeConflictsWithGlobalVariable",
        R"prg(
            int f;
            int f(void);
            int main() {
                return 0;
            }
        )prg",
        "conflicts with global variable",
    },
    SemanticErrorCase{
        "incompatibleFunctionRedeclaration",
        R"prg(
            int f(int);
            int f(double);
            int main() {
                return 0;
            }
        )prg",
        "declaration conflicts",
    }
), [](const testing::TestParamInfo<SemanticErrorCase> &info) { return std::string{info.param.name}; });

} // namespace
