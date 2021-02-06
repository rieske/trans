#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/FiniteAutomatonScanner.h"
#include "parser/LR1Parser.h"
#include "parser/FilePersistedParsingTable.h"
#include "parser/GeneratedParsingTable.h"
#include "parser/Action.h"
#include "driver/Configuration.h"
#include "driver/CompilerComponentsFactory.h"
#include "parser/SyntaxTreeBuilder.h"
#include "parser/LR1Strategy.h"
#include "parser/LALR1Strategy.h"

#include "ResourceHelpers.h"

using namespace testing;
using namespace parser;

namespace {

TEST(LR1Parser, parsesTestProgramUsingGeneratedLR1ParsingTable) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    configuration.setGrammarPath("resources/grammars/grammar_original.bnf");

    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
    ParsingTable* parsingTable = new GeneratedParsingTable(new BNFFileGrammar(getResourcePath("grammars/grammar_original.bnf")), LR1Strategy { });
    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder("test")));
}

TEST(LR1Parser, parsesTestProgramUsingGeneratedLALR1ParsingTable) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    configuration.setGrammarPath("resources/grammars/grammar_original.bnf");

    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
    ParsingTable* parsingTable = new GeneratedParsingTable(new BNFFileGrammar(getResourcePath("grammars/grammar_original.bnf")), LALR1Strategy { });
    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder("test")));
}

}
