#include "driver/Compiler.h"
#include "driver/TranslationUnit.h"
#include "driver/CompilerComponentsFactory.h"
#include "driver/Configuration.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "parser/SyntaxTree.h"
#include "parser/Parser.h"
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

class MockParser: public Parser {
public:
    std::unique_ptr<SyntaxTree> parse(Scanner& scanner, std::unique_ptr<SyntaxTreeBuilder> builder) override {
        return std::unique_ptr<SyntaxTree> { parseProxy(scanner, builder.get()) };
    }

    MOCK_METHOD2(parseProxy, SyntaxTree*(Scanner&, SyntaxTreeBuilder*));
};

TEST(Compiler, throwsForNonExistentFile) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    std::unique_ptr<MockParser> parser { new StrictMock<MockParser> };

    ASSERT_THROW(compiler.compile("nonexistentSourceFileName"), std::runtime_error);
}

TEST(Compiler, compilesFibonacciProgram) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    std::unique_ptr<MockParser> parser { new StrictMock<MockParser> };

    compiler.compile("test/programs/fibonacciRecursive.src");

    EXPECT_THAT(readFileContents("test/programs/fibonacciRecursive.src.S"), Eq(readFileContents("test/programs/expectedOutput/fibonacciRecursive.src.S")));
}

TEST(Compiler, compilesSwapProgram) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    std::unique_ptr<MockParser> parser { new StrictMock<MockParser> };

    compiler.compile("test/programs/swap.src");

    EXPECT_THAT(readFileContents("test/programs/swap.src.S"), Eq(readFileContents("test/programs/expectedOutput/swap.src.S")));
}

TEST(Compiler, compilesSimpleOutputProgram) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    std::unique_ptr<MockParser> parser { new StrictMock<MockParser> };

    compiler.compile("test/programs/simpleOutput.src");

    EXPECT_THAT(readFileContents("test/programs/simpleOutput.src.S"), Eq(readFileContents("test/programs/expectedOutput/simpleOutput.src.S")));
}

}
