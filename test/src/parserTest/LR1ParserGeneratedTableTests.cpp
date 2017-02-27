#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "src/scanner/FiniteAutomatonScanner.h"
#include "src/parser/LR1Parser.h"
#include "src/parser/FilePersistedParsingTable.h"
#include "src/parser/GeneratedParsingTable.h"
#include "src/parser/BNFFileGrammar.h"
#include "src/parser/Grammar.h"
#include "src/parser/Action.h"
#include "src/util/LogManager.h"
#include "src/driver/Configuration.h"
#include "src/driver/CompilerComponentsFactory.h"
#include "src/parser/SyntaxTree.h"
#include "src/parser/SyntaxTreeBuilder.h"
#include "src/parser/LR1Strategy.h"
#include "src/parser/LALR1Strategy.h"

#include <memory>

#include "ResourceHelpers.h"

using namespace testing;
using namespace parser;

using std::unique_ptr;

namespace {

class ConfigurationStub: public Configuration {
public:
    std::vector<std::string> getSourceFileNames() const {
        return {};
    }
    std::string getLexFileName() const {
        return getResourcePath("configuration/scanner.lex");
    }
    std::string getParsingTableFileName() const {
        return {};
    }
    std::string getGrammarFileName() const {
        return {};
    }
    bool usingCustomGrammar() const {
        return true;
    }
    bool isParserLoggingEnabled() const {
        return false;
    }
    bool isScannerLoggingEnabled() const {
        return false;
    }
};

TEST(LR1Parser, parsesTestProgramUsingGeneratedLR1ParsingTable) {
    ConfigurationStub configuration { };
    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
    ParsingTable* parsingTable = new GeneratedParsingTable(new BNFFileGrammar(getResourcePath("grammars/grammar_original.bnf")), LR1Strategy { });
    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder()));
}

TEST(LR1Parser, parsesTestProgramUsingGeneratedLALR1ParsingTable) {
    ConfigurationStub configuration { };
    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
    ParsingTable* parsingTable = new GeneratedParsingTable(new BNFFileGrammar(getResourcePath("grammars/grammar_original.bnf")), LALR1Strategy { });
    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder()));
}

}
