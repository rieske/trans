#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../scanner/FiniteAutomatonScanner.h"
#include "../parser/LR1Parser.h"
#include "../parser/FilePersistedParsingTable.h"
#include "../parser/GeneratedParsingTable.h"
#include "../parser/Action.h"
#include "../driver/Configuration.h"
#include "../driver/CompilerComponentsFactory.h"
#include "../parser/SyntaxTreeBuilder.h"
#include "../parser/LR1Strategy.h"
#include "../parser/LALR1Strategy.h"

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
