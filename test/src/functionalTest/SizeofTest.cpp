#include "TestFixtures.h"

namespace {

TEST(Compiler, sizeofTypeInt) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", sizeof(int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, sizeofTypeChar) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", sizeof(char));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, sizeofTypePointer) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", sizeof(int*));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, sizeofExpressionVariable) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            char c;
            int *p;
            printf("%d %d %d", sizeof x, sizeof c, sizeof p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 1 8");
}

TEST(Compiler, sizeofSizedArray) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            printf("%d", sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
}

TEST(Compiler, sizedArrayDeclarationAccepted) {
    // Declaration alone must type-check; no element access required.
    SourceProgram program{R"prg(
        int main() {
            int a[4];
            int b;
            b = 1;
            printf("%d", b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

} // namespace
