#include "TestFixtures.h"

#include "driver/CompilerComponentsFactory.h"
#include "driver/ConfigurationParser.h"

namespace {

TEST(Compiler, throwsForNonExistentFile) {
    std::string sourceFile = "nonexistentSourceFileName";
    std::vector<std::string> arguments {"trans", "-r../../../", sourceFile};
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    ConfigurationParser configurationParser { (int)argv.size()-1, argv.data() };
    Compiler compiler{ configurationParser.getConfiguration() };

    ASSERT_THROW(compiler.compile(sourceFile), std::runtime_error);
}

TEST(Compiler, reportsBasicParsingError) {
    SourceProgram program{R"prg(
        int main() {
            return 0 // missing semicolon
        }
    )prg"};

    program.compile();

    // FIXME: incorrectly reported line number, 5 is the last line in file, should be 3 instead
    program.assertCompilationErrors(":5: unexpected token: } expected: , ) : ; ]");
    program.assertCompilationErrors("Error: parsing failed with syntax errors");
}

}

