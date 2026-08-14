#include "TestFixtures.h"

// ISO C 6.9: a translation unit is one or more external declarations.
TEST(Compiler, emptyTranslationUnitIsSyntaxError) {
    SourceProgram program { "", { "-c" } };
    program.compile();
    program.assertCompilationErrors("unexpected token");
}

TEST(Compiler, whitespaceOnlyTranslationUnitIsSyntaxError) {
    SourceProgram program { "  \n\t\n", { "-c" } };
    program.compile();
    program.assertCompilationErrors("unexpected token");
}

TEST(Compiler, commentOnlyTranslationUnitIsSyntaxError) {
    SourceProgram program { "/* empty after preprocess */\n", { "-c" } };
    program.compile();
    program.assertCompilationErrors("unexpected token");
}
