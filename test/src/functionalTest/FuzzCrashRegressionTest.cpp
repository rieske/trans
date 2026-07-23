#include "TestFixtures.h"

namespace {

// Empty statement `;` must not abort the AST builder (null statement as statement list entry,
// if/while/for body, etc.). Found by mutation fuzzer — emptyStatement left the statement stack empty.

TEST(Compiler, emptyStatementInBlock) {
    SourceProgram program{R"prg(
        int main() {
            ;
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("");
}

TEST(Compiler, emptyStatementAsIfBody) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            if (a)
                ;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, emptyStatementAsWhileBody) {
    SourceProgram program{R"prg(
        int main() {
            while (0)
                ;
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, emptyStatementAsForBody) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            i = 0;
            for (i = 0; i < 3; i++)
                ;
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, infiniteForWithReturnExit) {
    // Empty for-header clauses must not abort the AST builder.
    SourceProgram program{R"prg(
        int main() {
            int i;
            i = 0;
            for (;;) {
                i = i + 1;
                if (i > 2) {
                    printf("%d", i);
                    return 0;
                }
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, forWithEmptyClauses) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            i = 0;
            for (; i < 2; ) {
                i = i + 1;
            }
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

// Parenthesized declarators: `(a)`, `(*p)`. Previously used parenthesizedExpression which
// only popped terminals and left DirectDeclarator stack empty → assert on popDirectDeclarator.

TEST(Compiler, parenthesizedDeclarator) {
    SourceProgram program{R"prg(
        int main() {
            int (a);
            a = 5;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, parenthesizedPointerDeclarator) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int (*p);
            a = 7;
            p = &a;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, doubleParenthesizedDeclarator) {
    SourceProgram program{R"prg(
        int main() {
            int ((a));
            a = 3;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

// Undeclared callee must be a semantic error, not an assertion in getResultSymbol().
// Fuzzer mutated printf → ntf / priatf / etc.

TEST(Compiler, undeclaredFunctionCallIsSemanticError) {
    SourceProgram program{R"prg(
        int main() {
            ntf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("symbol `ntf` is not defined");
}

TEST(Compiler, undeclaredFunctionCallDoesNotAbort) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            if (a) {
                printf("%d", 1);
                ntf("%d", 1);
            } else {
                printf("%d", 0);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("symbol `ntf` is not defined");
}

TEST(Compiler, callNonFunctionIsSemanticError) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            a(2);
            return 0;
        }
    )prg"};
    program.compile();
    // Diagnostic must use the source identifier, not the scope-mangled symbol table name.
    program.assertCompilationErrors("called object `a` is not a function");
}

// A local must not resolve to a same-named function when used as a callee.
TEST(Compiler, localShadowsFunctionNotCallable) {
    SourceProgram program{R"prg(
        int a(int x) {
            return x;
        }
        int main() {
            int a;
            a = 1;
            a(2);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("called object `a` is not a function");
}

// Abstract parameters (`int f(int)`) must not null-deref FormalArgument::visitDeclarator.
// ASAN SEGV found by targeted probing after the main fuzzer run.

TEST(Compiler, abstractParameterInDefinition) {
    SourceProgram program{R"prg(
        int f(int) {
            return 1;
        }
        int main() {
            printf("%d", f(0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, abstractParametersMultiple) {
    SourceProgram program{R"prg(
        int add(int, int) {
            return 3;
        }
        int main() {
            printf("%d", add(1, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

// Multiple abstract parameters must each get a distinct argument slot (ABI arity).
// Named parameters that are actually used cover the value path; empty-name collisions
// are covered by SymbolTable.abstractArgumentNamesPreserveArity.

TEST(Compiler, namedParametersMultipleUsesValues) {
    SourceProgram program{R"prg(
        int add(int a, int b) {
            return a + b;
        }
        int main() {
            printf("%d", add(1, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, abstractPointerParameter) {
    SourceProgram program{R"prg(
        int f(int *) {
            return 1;
        }
        int main() {
            printf("%d", f(0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Nested assignment with a non-lvalue LHS must not abort. C 6.2.1: `a` is in scope
// in its own initializer; the error is that `(+a)` is not an lvalue (same as gcc).

TEST(Compiler, useInOwnInitializerIsSemanticErrorNotAbort) {
    SourceProgram program{R"prg(
        int main() {
            int a = (+a) = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("lvalue required");
}

// C 6.2.1: the declarand is in scope for its initializer (value is indeterminate for
// `int a = a`, but not a constraint error). Address self-init is well-defined.

TEST(Compiler, valueSelfReferenceInInitializerIsInScope) {
    SourceProgram program{R"prg(
        int main() {
            int a = a;
            void *p = &p;
            printf("%d", p == &p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, undefinedInUnaryPlusIsSemanticError) {
    SourceProgram program{R"prg(
        int main() {
            int b;
            b = +nope;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("symbol `nope` is not defined");
}

// ArrayAccess must recover when the left operand failed to resolve — not throw "type is not set".

TEST(Compiler, undeclaredArrayAccessIsSemanticErrorNotAbort) {
    SourceProgram program{R"prg(
        int main() {
            undeclared[0];
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("symbol `undeclared` is not defined");
    program.assertCompilationErrors("Semantic errors were detected");
}

// `int (*f)()` must type as pointer-to-function so unary * is valid on f.

TEST(Compiler, parenthesizedFunctionPointerDeclarator) {
    SourceProgram program{R"prg(
        int main() {
            int (*f)();
            int x;
            x = 0;
            f = 0;
            if (x)
                *f;
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("");
}

// Conditional jumps must spill live registers (including arg homes) on the branch edge.
// Previously only fall-through into label() spilled, so `if (0); return a+b` used unsaved args.
// Found by mutation fuzzer (semantics-preserving dead if / empty statement injection).

TEST(Compiler, ifFalseBeforeReturnUsesArguments) {
    SourceProgram program{R"prg(
        int add(int a, int b) {
            if (0) {
                ;
            }
            return a + b;
        }
        int main() {
            printf("%d", add(1, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, ifFalseBeforeReturnUsesSingleArgument) {
    SourceProgram program{R"prg(
        int id(int x) {
            if (0) { ; }
            return x;
        }
        int main() {
            printf("%d", id(42));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, ifTrueReturnArgumentElseOther) {
    SourceProgram program{R"prg(
        int pick(int a, int b) {
            if (a) {
                return a;
            }
            return b;
        }
        int main() {
            printf("%d ", pick(0, 7));
            printf("%d", pick(3, 7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 3");
}

TEST(Compiler, deadBlockInCalleeDoesNotClobberReturn) {
    SourceProgram program{R"prg(
        int add(int a, int b) {
            if (0) { ; }
            return a + b;
        }
        int main() {
            { int dead; dead = 0; }
            printf("%d", add(1, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

// Function designators decay to pointer-to-function (C 6.3.2.1); codegen emits FunctionAddress.
// Bare designators used to throw map::at in codegen when treated as values without address.
TEST(Compiler, functionDesignatorAsValueDecaysToPointer) {
    SourceProgram program{R"prg(
        int main() {
            void *p;
            p = main;
            printf("%d", p != 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Fuzzer bucket: grammar accepts K&R productions that the AST builder does not
// implement. These must report a clear "not implemented" error, not
// "no AST creator defined for production".

TEST(Compiler, knrIdentifierParameterListIsNotImplemented) {
    SourceProgram program{R"prg(
        int f(a) {
            return 0;
        }
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("K&R identifier parameter lists is not implemented yet");
}

TEST(Compiler, knrFunctionDefinitionIsNotImplemented) {
    // Full K&R form also requires id_list on the declarator; either message is fine.
    SourceProgram program{R"prg(
        int f(a)
        int a;
        {
            return a;
        }
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("K&R");
}


// Signed >> must use SAR (arithmetic), not SHR (logical). Fuzzer pure-expr oracle:
// (~7)>>2 is -2, so (~7)>>2 > 1 is false — with SHR it became a huge positive.

TEST(Compiler, arithmeticShiftRightPreservesSign) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d ", ~7);
            printf("%d ", (~7) >> 2);
            printf("%d ", ((~7) >> 2) > 1);
            printf("%d", (-8) >> 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-8 -2 0 -4");
}


} // namespace
