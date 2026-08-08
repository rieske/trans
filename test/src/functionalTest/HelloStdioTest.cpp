#include "TestFixtures.h"

#include <filesystem>

namespace {

void expectPreprocessTempRemoved(const Program& program) {
    EXPECT_FALSE(std::filesystem::exists(program.getSourceFilePath() + ".i"));
}

// Product path: a real TU with #include <stdio.h>, no implicit printf, gcc -E, link, run.

TEST(Compiler, helloWithStdioInclude) {
    SourceProgram program { R"prg(#include <stdio.h>
        int main(void) {
            printf("hello\n");
            return 0;
        }
    )prg"};

    program.compile();
    expectPreprocessTempRemoved(program);
    program.runAndExpect("hello\n");
}

TEST(Compiler, printfWithoutDeclarationIsError) {
    SourceProgram program { R"prg(
        int main(void) {
            printf("hello\n");
            return 0;
        }
    )prg"};

    program.compile();
    expectPreprocessTempRemoved(program);
    program.assertCompilationErrors("symbol `printf` is not defined");
}

} // namespace
