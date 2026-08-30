#include "gtest/gtest.h"

#include "scanner/LexicalSession.h"
#include "gmock/gmock.h"

#include "parser/LR1Parser.h"
#include "parser/BNFFileReader.h"
#include "parser/Grammar.h"
#include "parser/ParsingTable.h"
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

    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });

    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
    ParsingTable parsingTable { &grammar };

    LR1Parser parser { parsingTable };
    scanner::LexicalSession session;
    auto builder = compilerComponentsFactory.makeSyntaxTreeBuilder(&grammar, session);
    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(
                    getTestResourcePath("programs/example_prog.c"), session),
                    *builder));
}

}
