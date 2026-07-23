#include "TestFixtures.h"

namespace {

TEST(SemanticErrors, voidVariable) {
    SourceProgram program{R"prg(
        int main() {
            void a;
            return 0;
        }
    )prg"};

    program.compile();

    program.assertCompilationErrors(":3: error: variable `a` declared void");
}

TEST(SemanticErrors, undeclaredIdentifier) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", noSuchVariable);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors(":3: error: symbol `noSuchVariable` is not defined");
}

TEST(SemanticErrors, assignToRvalueRejected) {
    SourceProgram program{R"prg(
        int main() {
            3 = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors(":3: error: lvalue required on the left side of assignment");
}

TEST(SemanticErrors, assignToUnaryPlusRejected) {
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

TEST(SemanticErrors, breakOutsideLoopOrSwitch) {
    SourceProgram program{R"prg(
        int main() {
            break;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("break statement not within a loop or switch");
}

TEST(SemanticErrors, continueOutsideLoop) {
    SourceProgram program{R"prg(
        int main() {
            continue;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("continue statement not within a loop or switch");
}

TEST(SemanticErrors, caseOutsideSwitch) {
    SourceProgram program{R"prg(
        int main() {
            case 1:
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("case label not within a switch statement");
}

TEST(SemanticErrors, gotoUndefinedLabel) {
    SourceProgram program{R"prg(
        int main() {
            goto missing;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("label `missing` used but not defined");
}

TEST(SemanticErrors, memberOfNonStruct) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x.y = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("request for member in non-struct");
}

TEST(SemanticErrors, unknownStructMember) {
    SourceProgram program{R"prg(
        struct S { int a; };
        int main() {
            struct S s;
            s.nope = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("struct has no member named `nope`");
}

TEST(SemanticErrors, negativeArraySize) {
    SourceProgram program{R"prg(
        int main() {
            int a[-1];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is negative");
}

TEST(SemanticErrors, duplicateLabel) {
    SourceProgram program{R"prg(
        int main() {
            again:
            again:
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("duplicate label `again`");
}

TEST(SemanticErrors, calledObjectNotFunction) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = 1;
            x();
            return 0;
        }
    )prg"};
    program.compile();
    // Prefer the source name in the diagnostic (not a scope-mangled symbol-table key).
    program.assertCompilationErrors("called object `x` is not a function");
}

TEST(SemanticErrors, wrongArityNoMatchForFunction) {
    SourceProgram program{R"prg(
        int add(int a, int b) {
            return a + b;
        }
        int main() {
            printf("%d", add(1));
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("no match for function");
}

TEST(SemanticErrors, functionConflictsWithGlobalVariable) {
    SourceProgram program{R"prg(
        int foo;
        int foo(void) {
            return 1;
        }
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("conflicts with global variable");
}

TEST(SemanticErrors, symbolConflictsWithPreviousDeclaration) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            int x;
            x = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("conflicts with previous declaration");
}

TEST(SemanticErrors, multipleDefaultLabelsInSwitch) {
    SourceProgram program{R"prg(
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
    )prg"};
    program.compile();
    program.assertCompilationErrors("multiple default labels in switch");
}

TEST(SemanticErrors, caseLabelNotConstantExpression) {
    SourceProgram program{R"prg(
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
    )prg"};
    program.compile();
    program.assertCompilationErrors("case label is not a constant expression");
}

TEST(SemanticErrors, voidFunctionArgumentRejected) {
    SourceProgram program{R"prg(
        int f(void x) {
            return 0;
        }
        int main() {
            return f();
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("declared void");
}

TEST(SemanticErrors, wrongNumberOfArgumentsToVaStart) {
    // va_start accepts 1 or 2 args; zero is always invalid.
    SourceProgram program{R"prg(
        int f(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_start();
            __builtin_va_end(ap);
            return n;
        }
        int main() {
            return f(1, 2);
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("wrong number of arguments to");
}

TEST(SemanticErrors, wrongNumberOfArgumentsToVaCopy) {
    SourceProgram program{R"prg(
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
    )prg"};
    program.compile();
    program.assertCompilationErrors("wrong number of arguments to __builtin_va_copy");
}

TEST(SemanticErrors, baseOfArrowIsNotPointer) {
    SourceProgram program{R"prg(
        struct S { int a; };
        int main() {
            struct S s;
            s.a = 1;
            printf("%d", s->a);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("base of '->' is not a pointer");
}

// Struct and scalar are not assignment-compatible (productCanAssignFrom rejects).
TEST(SemanticErrors, assignStructToScalarRejected) {
    SourceProgram program{R"prg(
        struct S { int a; };
        int main() {
            struct S s;
            int x;
            s.a = 1;
            x = s;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// Same rule for initialization (DeclarationAnalyzer type-checks initializers).
TEST(SemanticErrors, structInitializerToScalarRejected) {
    SourceProgram program{R"prg(
        struct S { int a; };
        int main() {
            struct S s;
            s.a = 1;
            int x = s;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

TEST(SemanticErrors, scalarInitializerToStructRejected) {
    SourceProgram program{R"prg(
        struct S { int a; };
        int main() {
            struct S s = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// Regression: glibc bind/accept take a transparent union of sockaddr pointers
// (__CONST_SOCKADDR_ARG). Passing `struct sockaddr *` must type-check (git daemon).
TEST(Compiler, sockaddrPointerArgToTransparentUnionParam) {
    SourceProgram program{R"prg(
        struct sockaddr { int family; };
        typedef union {
            struct sockaddr *__restrict __sockaddr__;
        } __SOCKADDR_ARG;
        int accept(int fd, __SOCKADDR_ARG addr, int *len);
        int main() {
            struct sockaddr sa;
            int len;
            len = 0;
            sa.family = 2;
            accept(0, &sa, &len);
            printf("%d", sa.family);
            return 0;
        }
    )prg", "sockaddr_transparent_union"};
    program.compile();
    program.runAndExpect("2");
}

} // namespace

