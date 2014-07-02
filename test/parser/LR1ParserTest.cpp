#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/FiniteAutomatonScanner.h"
#include "parser/LR1Parser.h"
#include "parser/FilePersistedParsingTable.h"
#include "parser/GeneratedParsingTable.h"
#include "parser/BNFFileGrammar.h"
#include "parser/Grammar.h"
#include "parser/Action.h"
#include "util/LogManager.h"
#include "driver/Configuration.h"
#include "driver/CompilerComponentsFactory.h"
#include "parser/SyntaxTree.h"

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
	LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });

	ParsingTable* parsingTable = new FilePersistedParsingTable("resources/configuration/parsing_table",
			new BNFFileGrammar("resources/configuration/grammar.bnf"));

	LR1Parser parser { parsingTable };

	unique_ptr<SyntaxTree> syntaxTree = parser.parse(*compilerComponentsFactory.scannerForSourceFile("test/programs/example_prog.src"),
			*compilerComponentsFactory.newSemanticAnalyzer());

	ASSERT_THAT(syntaxTree, NotNull());
}

TEST(LR1Parser, parsesTestProgramUsingGeneratedParsingTable) {
	MockConfiguration configuration { };
	CompilerComponentsFactory compilerComponentsFactory { configuration };
	LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
	ParsingTable* parsingTable = new GeneratedParsingTable(new BNFFileGrammar("resources/configuration/grammar.bnf"));
	LR1Parser parser { parsingTable };

	unique_ptr<SyntaxTree> syntaxTree = parser.parse(*compilerComponentsFactory.scannerForSourceFile("test/programs/example_prog.src"),
			*compilerComponentsFactory.newSemanticAnalyzer());

	ASSERT_THAT(syntaxTree, NotNull());
}
