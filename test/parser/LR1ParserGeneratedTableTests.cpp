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
#include "parser/SyntaxTreeBuilder.h"
#include "parser/LR1Strategy.h"
#include "parser/LALR1Strategy.h"

#include <memory>

using namespace testing;
using namespace parser;

using std::unique_ptr;

namespace {

class ConfigurationStub: public Configuration {
public:
    virtual const std::vector<std::string> getSourceFileNames() const {
        return {};
    }
    virtual const std::string getCustomGrammarFileName() const {
        return {};
    }
    virtual bool usingCustomGrammar() const {
        return true;
    }
    virtual bool isParserLoggingEnabled() const {
        return true;
    }
    virtual bool isScannerLoggingEnabled() const {
        return false;
    }
};

TEST(LR1Parser, parsesTestProgramUsingGeneratedLR1ParsingTable) {
    ConfigurationStub configuration { };
    CompilerComponentsFactory compilerComponentsFactory { configuration };
    LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
    ParsingTable* parsingTable = new GeneratedParsingTable(new BNFFileGrammar("resources/grammars/grammar_original.bnf"), LR1Strategy { });
    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.scannerForSourceFile("test/programs/example_prog.src"),
                    compilerComponentsFactory.newSyntaxTreeBuilder()));
}

TEST(LR1Parser, parsesTestProgramUsingGeneratedLALR1ParsingTable) {
    ConfigurationStub configuration { };
    CompilerComponentsFactory compilerComponentsFactory { configuration };
    LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
    ParsingTable* parsingTable = new GeneratedParsingTable(new BNFFileGrammar("resources/grammars/grammar_original.bnf"), LALR1Strategy { });
    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.scannerForSourceFile("test/programs/example_prog.src"),
                    compilerComponentsFactory.newSyntaxTreeBuilder()));
}

}
