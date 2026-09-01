#include "TestFixtures.h"

#include "driver/ConfigurationParser.h"
#include "util/LogManager.h"

#include <optional>
#include <sstream>

namespace {

TEST(Compiler, reportsMissingSourceFile) {
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

    std::stringstream outputStream;
    std::stringstream errorStream;
    std::optional<std::string> result;
    LogManager::withOutputStreams(outputStream, errorStream, [&]() {
        result = compiler.compile(sourceFile);
    });
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(errorStream.str(), HasSubstr("nonexistentSourceFileName:0: error:"));
}

TEST(Compiler, reportsBasicParsingError) {
    SourceProgram program{R"prg(
        int main() {
            return 0 // missing semicolon
        }
)prg"};

    program.compile();

    program.assertCompilationErrors(":4: error: unexpected token: } expected: , ) : ; ]");
    program.assertCompilationErrors("Error: parsing failed with syntax errors");
}

}

