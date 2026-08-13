#include "TestFixtures.h"

namespace {

// C: incomplete object types are allowed for pure extern declarations
// (no storage in this TU). Git: `extern struct trace_key trace_shallow;`
// while the full struct is only completed where needed.

TEST(Compiler, externIncompleteStructDeclarationIsOk) {
    SourceProgram program{R"prg(
        struct S;
        extern struct S x;
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("");
}

TEST(Compiler, externIncompleteStructThenCompleteDefinition) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S;
        extern struct S x;
        struct S { int a; };
        struct S x = { 42 };
        int main(void) {
            printf("%d", x.a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, externIncompleteStructAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S;
        extern struct S x;
        struct S { int a; };
        struct S x = { 7 };
        int main(void) {
            printf("%d", (int)((long)&x != 0) + x.a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, externIncompleteUnionDeclarationIsOk) {
    SourceProgram program{R"prg(
        union U;
        extern union U u;
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("");
}

// Block-scope extern of incomplete type is a pure declaration (no frame home).
TEST(Compiler, localExternIncompleteStructDeclIsOk) {
    SourceProgram program{R"prg(
        struct S;
        int f(void) {
            extern struct S x;
            return 0;
        }
        int main(void) {
            return f();
        }
    )prg"};
    program.compile();
    program.runAndExpect("");
}

// Primary regression: block-scope extern must load the defining object, not a stack slot.
TEST(Compiler, localExternReadsGlobal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = 42;
        int f(void) {
            extern int g;
            return g;
        }
        int main(void) {
            printf("%d", f());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Complete type at the local extern; load must hit the defining object, not a frame home.
TEST(Compiler, localExternStructReadsDefiningObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int a; };
        struct S x = { 5 };
        int f(void) {
            extern struct S x;
            return x.a;
        }
        int main(void) {
            printf("%d", f());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

// Still incomplete where storage size is required.

TEST(Compiler, incompleteStructFileScopeDefinitionIsError) {
    SourceProgram program{R"prg(
        struct S;
        struct S x;
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("incomplete type");
}

TEST(Compiler, incompleteStructStaticIsError) {
    SourceProgram program{R"prg(
        struct S;
        static struct S x;
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("incomplete type");
}

TEST(Compiler, incompleteStructLocalObjectIsError) {
    SourceProgram program{R"prg(
        struct S;
        int main(void) {
            struct S s;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("incomplete type");
}

TEST(Compiler, sizeofIncompleteExternStructIsError) {
    SourceProgram program{R"prg(
        struct S;
        extern struct S x;
        int main(void) {
            return (int)sizeof(x);
        }
    )prg"};
    program.compile();
    // After allowing the extern decl, sizeof must still reject incomplete objects.
    program.assertCompilationErrors("incomplete");
}

TEST(Compiler, incompleteExternWithInitializerIsError) {
    SourceProgram program{R"prg(
        struct S;
        extern struct S x = { 0 };
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("incomplete type");
}

} // namespace
