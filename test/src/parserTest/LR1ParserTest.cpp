#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/LR1Parser.h"
#include "parser/FilePersistedParsingTable.h"
#include "parser/BNFFileGrammar.h"
#include "parser/Grammar.h"
#include "util/LogManager.h"
#include "driver/Configuration.h"
#include "driver/CompilerComponentsFactory.h"
#include "parser/SyntaxTreeBuilder.h"

#include "ResourceHelpers.h"

#include <memory>

using namespace testing;
using namespace parser;

namespace {

TEST(LR1Parser, parsesTestProgram) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    configuration.setGrammarPath("resources/configuration/grammar.bnf");

    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });

    BNFFileGrammar reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
    ParsingTable* parsingTable = new FilePersistedParsingTable(getResourcePath("configuration/parsing_table"), &grammar);

    LR1Parser parser { parsingTable };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder("test", &grammar)));
}

}
