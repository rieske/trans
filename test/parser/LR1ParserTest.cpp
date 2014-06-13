#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/FiniteAutomatonScanner.h"
#include "parser/LR1Parser.h"
#include "parser/ParsingTable.h"
#include "util/Logger.h"
#include "driver/Configuration.h"
#include "driver/CompilerComponentsFactory.h"
#include "semantic_analyzer/SemanticComponentsFactory.h"
#include "semantic_analyzer/SyntaxTree.h"

#include <memory>

using namespace testing;
using std::unique_ptr;

class MockConfiguration: public Configuration {
public:
	MOCK_CONST_METHOD0(getSourceFileNames, const std::vector<std::string> &());
	MOCK_CONST_METHOD0(getCustomGrammarFileName, const std::string ());
	MOCK_CONST_METHOD0(usingCustomGrammar, bool());
	MOCK_CONST_METHOD0(isParserLoggingEnabled, bool ());
	MOCK_CONST_METHOD0(isScannerLoggingEnabled, bool ());
};

TEST(LR1Parser, parsesTestProgram) {
	MockConfiguration configuration { };
	CompilerComponentsFactory compilerComponentsFactory { configuration };
	Logger logger { std::cerr };
	LR1Parser parser { new ParsingTable { logger }, new SemanticComponentsFactory { true }, logger };

	unique_ptr<SyntaxTree> syntaxTree = parser.parse(*compilerComponentsFactory.getScanner("test/programs/example_prog.src"));

	ASSERT_THAT(syntaxTree, NotNull());
}

TEST(LR1Parser, parsesTestProgramUsingGeneratedParsingTable) {
	MockConfiguration configuration { };
	CompilerComponentsFactory compilerComponentsFactory { configuration };
	Logger logger { std::cerr };
	LR1Parser parser { new ParsingTable { "grammar.bnf", logger }, new SemanticComponentsFactory { true }, logger };

	unique_ptr<SyntaxTree> syntaxTree = parser.parse(*compilerComponentsFactory.getScanner("test/programs/example_prog.src"));

	ASSERT_THAT(syntaxTree, NotNull());
}
