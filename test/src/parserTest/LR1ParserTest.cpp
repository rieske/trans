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

#include "ResourceHelpers.h"

#include <memory>

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

TEST(LR1Parser, parsesTestProgram) {
    CompilerComponentsFactory compilerComponentsFactory { std::make_unique<ConfigurationStub>() };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });

    ParsingTable* parsingTable = new FilePersistedParsingTable(getResourcePath("configuration/parsing_table"),
            new BNFFileGrammar(getResourcePath("configuration/grammar.bnf")));

    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder()));
}

}
