#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileGrammar.h"
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
    std::unique_ptr<Grammar> grammar = std::make_unique<BNFFileGrammar>(getResourcePath("grammars/grammar_original.bnf"));
    ParsingTable* parsingTable = new GeneratedParsingTable(grammar.get(), LR1Strategy{});
    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder("test", grammar.get())));
}

TEST(LR1Parser, parsesTestProgramUsingGeneratedLALR1ParsingTable) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    configuration.setGrammarPath("resources/grammars/grammar_original.bnf");

    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
    std::unique_ptr<Grammar> grammar = std::make_unique<BNFFileGrammar>(getResourcePath("grammars/grammar_original.bnf"));
    ParsingTable* parsingTable = new GeneratedParsingTable(grammar.get(), LALR1Strategy {});
    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder("test", grammar.get())));
}

}
