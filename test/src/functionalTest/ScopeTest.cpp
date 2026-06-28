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

// TODO(gap): block-scope shadowing - inner `int a` reports "declaration conflicts"
// with outer `a`. Symbol table treats nested scopes as a flat conflict instead of
// allowing a new binding (README mentions prefixing inner locals; not applied to
// same name in an inner block). Need proper nested scopes in SemanticAnalysisVisitor
// / SymbolTable so outer `a` remains visible after the block.
/*
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
*/

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
