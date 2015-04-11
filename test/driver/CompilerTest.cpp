#include "driver/Compiler.h"
#include "driver/TranslationUnit.h"
#include "driver/CompilerComponentsFactory.h"
#include "driver/Configuration.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>
#include <fstream>
#include <streambuf>

using namespace testing;
using namespace parser;

namespace {

std::string readFileContents(std::string filename) {
    std::ifstream inputStream(filename);
    std::string content;

    inputStream.seekg(0, std::ios::end);
    content.reserve(inputStream.tellg());
    inputStream.seekg(0, std::ios::beg);

    content.assign((std::istreambuf_iterator<char>(inputStream)), std::istreambuf_iterator<char>());
    return content;
}

class MockConfiguration: public Configuration {
public:
    MOCK_CONST_METHOD0(getSourceFileNames, const std::vector<std::string>());
    MOCK_CONST_METHOD0(getCustomGrammarFileName, const std::string ());
    MOCK_CONST_METHOD0(usingCustomGrammar, bool());
    MOCK_CONST_METHOD0(isParserLoggingEnabled, bool ());
    MOCK_CONST_METHOD0(isScannerLoggingEnabled, bool ());
};

TEST(Compiler, throwsForNonExistentFile) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    ASSERT_THROW(compiler.compile("nonexistentSourceFileName"), std::runtime_error);
}

TEST(Compiler, compilesFibonacciProgram) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    compiler.compile("test/programs/fibonacciRecursive.src");

    system("rm test/programs/fibonacciRecursive.execution.output");
    system("echo \"42\" | test/programs/fibonacciRecursive.src.out > test/programs/fibonacciRecursive.execution.output");

    std::string expectedOutput { "1\n"
            "2\n"
            "3\n"
            "5\n"
            "8\n"
            "13\n"
            "21\n"
            "34\n"
            "55\n"
    };
    EXPECT_THAT(readFileContents("test/programs/fibonacciRecursive.execution.output"), Eq(expectedOutput));
}

TEST(Compiler, compilesSwapProgram) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    compiler.compile("test/programs/swap.src");

    system("rm test/programs/swap.execution.output");
    system("test/programs/swap.src.out > test/programs/swap.execution.output");

    std::ifstream expectedOutputStream { "test/programs/swap.execution.output" };
    std::string outputLine;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    std::string firstAddressBefore;
    expectedOutputStream >> firstAddressBefore;
    std::string secondAddressBefore;
    expectedOutputStream >> secondAddressBefore;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    std::string firstAddressAfter;
    expectedOutputStream >> firstAddressAfter;
    std::string secondAddressAfter;
    expectedOutputStream >> secondAddressAfter;
    EXPECT_THAT(firstAddressBefore, Not(Eq(secondAddressBefore)));
    EXPECT_THAT(firstAddressBefore, Eq(firstAddressAfter));
    EXPECT_THAT(secondAddressBefore, Eq(secondAddressAfter));
}

TEST(Compiler, compilesSimpleOutputProgram) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    compiler.compile("test/programs/simpleOutput.src");

    system("rm test/programs/simpleOutput.execution.output");
    system("test/programs/simpleOutput.src.out > test/programs/simpleOutput.execution.output");

    std::string expectedOutput { "1\n"
            "-1\n"
            "1\n"
            "-3\n"
    };
    EXPECT_THAT(readFileContents("test/programs/simpleOutput.execution.output"), Eq(expectedOutput));
}

}
