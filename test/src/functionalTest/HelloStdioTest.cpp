#include "TestFixtures.h"

namespace {

// Product path: a real TU with #include <stdio.h>, no implicit printf, gcc -E, link, run.

TEST(Compiler, helloWithStdioInclude) {
    SourceProgram program { R"prg(#include <stdio.h>
        int main(void) {
            printf("hello\n");
            return 0;
        }
    )prg"};

    program.compile();
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
    program.assertCompilationErrors("symbol `printf` is not defined");
}

} // namespace
