#include "src/driver/Compiler.h"
#include "src/translation_unit/TranslationUnit.h"
#include "src/driver/CompilerComponentsFactory.h"
#include "src/driver/Configuration.h"
#include "src/semantic_analyzer/SemanticAnalyzer.h"
#include "src/scanner/Scanner.h"
#include "src/scanner/Token.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>
#include <fstream>
#include <streambuf>

#include "ResourceHelpers.h"

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

void callSystem(std::string command) {
    system(command.c_str());
}

class MockConfiguration: public Configuration {
public:
	MockConfiguration() {}
	virtual ~MockConfiguration() {}

	std::vector<std::string> getSourceFileNames() const override { return {}; }
	std::string getGrammarFileName() const override { return getResourcePath("configuration/grammar.bnf"); }
    std::string getParsingTableFileName() const override { return getResourcePath("configuration/parsing_table"); }
    std::string getLexFileName() const override { return getResourcePath("configuration/scanner.lex"); }
	bool usingCustomGrammar() const override { return false; }
	bool isParserLoggingEnabled() const override { return false; }
	bool isScannerLoggingEnabled() const override { return false; }
};

TEST(Compiler, throwsForNonExistentFile) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    ASSERT_THROW(compiler.compile("nonexistentSourceFileName"), std::runtime_error);
}

TEST(Compiler, compilesFibonacciProgram) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    callSystem("rm fibonacciRecursive.execution.output " + getTestResourcePath("programs/fibonacciRecursive.src.out"));

    compiler.compile(getTestResourcePath("programs/fibonacciRecursive.src"));

    callSystem("echo \"42\" | " + getTestResourcePath("programs/fibonacciRecursive.src.out") + " > fibonacciRecursive.execution.output");

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
    EXPECT_THAT(readFileContents("fibonacciRecursive.execution.output"), Eq(expectedOutput));
}

TEST(Compiler, compilesSwapProgram) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    callSystem("rm swap.execution.output " + getTestResourcePath("programs/swap.src.out"));

    compiler.compile(getTestResourcePath("programs/swap.src"));

    callSystem(getTestResourcePath("programs/swap.src.out") + " > swap.execution.output");

    std::ifstream expectedOutputStream { "swap.execution.output" };
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

    callSystem("rm simpleOutput.execution.output " + getTestResourcePath("programs/simpleOutput.src.out"));

    compiler.compile(getTestResourcePath("programs/simpleOutput.src"));

    callSystem(getTestResourcePath("programs/simpleOutput.src.out") + " > simpleOutput.execution.output");

    std::string expectedOutput { "1\n"
            "-1\n"
            "1\n"
            "-3\n"
    };
    EXPECT_THAT(readFileContents("simpleOutput.execution.output"), Eq(expectedOutput));
}

}
