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

using namespace testing;
using namespace parser;

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

	try {
		compiler.compile("test/programs/test_prog1");
		FAIL();
	} catch (std::runtime_error& err) {
		ASSERT_THAT(err.what(), StrEq("Error creating assembler output file!"));
	}
}

TEST(Compiler, compilesSwapProgram) {
    MockConfiguration configuration;
    Compiler compiler { new CompilerComponentsFactory { configuration } };

    std::unique_ptr<MockParser> parser { new StrictMock<MockParser> };

    try {
        compiler.compile("test/programs/test_prog2");
        FAIL();
    } catch (std::runtime_error& err) {
        ASSERT_THAT(err.what(), StrEq("Error creating assembler output file!"));
    }
}
