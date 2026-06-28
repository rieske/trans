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
    program.assertCompilationErrors(":3: error: symbol `noSuchVariable` is not defined");
}

TEST(Compiler, assignToRvalueRejected) {
    SourceProgram program{R"prg(
        int main() {
            3 = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors(":3: error: lvalue required on the left side of assignment");
}

TEST(Compiler, assignToUnaryPlusRejected) {
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

} // namespace

