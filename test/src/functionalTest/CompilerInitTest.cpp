#include "TestFixtures.h"

#include "driver/ConfigurationParser.h"

namespace {

TEST(Compiler, throwsForNonExistentFile) {
    std::string sourceFile = "nonexistentSourceFileName";
    std::vector<std::string> arguments {"trans", "--resources=../../../", sourceFile};
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    ParseResult parsed = parseCommandLine((int)argv.size() - 1, argv.data());
    ASSERT_TRUE(parsed.configuration.has_value()) << parsed.message;
    Compiler compiler { *parsed.configuration };

    ASSERT_THROW(compiler.compile(sourceFile), std::runtime_error);
}

TEST(Compiler, reportsBasicParsingError) {
    SourceProgram program{R"prg(
        int main() {
            return 0 // missing semicolon
        }
)prg"};

    program.compile();

    program.assertCompilationErrors(":4: unexpected token: } expected: , ) : ; ]");
    program.assertCompilationErrors("Error: parsing failed with syntax errors");
}

}

