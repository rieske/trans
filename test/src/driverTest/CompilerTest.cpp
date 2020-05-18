#include "driver/Compiler.h"
#include "translation_unit/TranslationUnit.h"
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

    content.assign(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>());
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

std::string compile(std::string programName) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    std::string executableFile = getTestResourcePath("programs/" + programName + ".src.out");
    callSystem("rm " + executableFile);

    compiler.compile(getTestResourcePath("programs/" + programName + ".src"));
    return executableFile;
}

std::string compileAndRun(std::string programName) {
    std::string executableFile = compile(programName);
    std::string outputFile = "execution.output";
    callSystem("rm " + outputFile);
    callSystem(executableFile + " > " + outputFile);
    return outputFile;
}

std::string compileAndRun(std::string programName, std::string input) {
    std::string executableFile = compile(programName);
    std::string outputFile = "execution.output";
    callSystem("rm " + outputFile);
    callSystem("echo '" + input + "' | " + executableFile + " > " + outputFile);
    return outputFile;
}

void testProgram(std::string programName, std::string expectedOutput) {
    auto outputFile = compileAndRun(programName);
    EXPECT_THAT(readFileContents(outputFile), Eq(expectedOutput));
}

void testProgram(std::string programName, std::string input, std::string expectedOutput) {
    auto outputFile = compileAndRun(programName, input);
    EXPECT_THAT(readFileContents(outputFile), Eq(expectedOutput));
}

TEST(Compiler, throwsForNonExistentFile) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    ASSERT_THROW(compiler.compile("nonexistentSourceFileName"), std::runtime_error);
}

TEST(Compiler, compilesFibonacciProgram) {
    std::string expectedOutput { "1\n2\n3\n5\n8\n13\n21\n34\n55\n" };
    testProgram("fibonacciRecursive", "42", expectedOutput);
}

TEST(Compiler, compilesSwapProgram) {
    std::string outputFileName = compileAndRun("swap");

    std::ifstream expectedOutputStream { outputFileName };
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
    testProgram("simpleOutput", "1\n-1\n1\n-3\n");
}

TEST(Compiler, compilesLoopsProgram) {
    testProgram("loops/while", "120\n10\n");
}

}
