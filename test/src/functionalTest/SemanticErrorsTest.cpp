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

} // namespace

