#include "TestFixtures.h"

namespace {

TEST(Compiler, voidVariable) {
    SourceProgram program{R"prg(
        int main() {
            void a;
            return 0;
        }
    )prg"};

    program.compile();

    program.assertCompilationErrors(":3: error: variable `a` declared void");
}

TEST(Compiler, undeclaredIdentifier) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", noSuchVariable);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("error");
}

TEST(Compiler, assignToRvalueRejected) {
    SourceProgram program{R"prg(
        int main() {
            3 = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("error");
}

} // namespace

