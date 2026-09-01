#include "gtest/gtest.h"

#include "scanner/LexicalSession.h"
#include "gmock/gmock.h"

#include "parser/LR1Parser.h"
#include "parser/BNFFileReader.h"
#include "parser/Grammar.h"
#include "parser/ParsingTable.h"
#include "util/LogManager.h"
#include "ast/AbstractSyntaxTreeBuilder.h"
#include "driver/Configuration.h"
#include "scanner/LexFileScannerReader.h"
#include "scanner/Scanner.h"

#include "ResourceHelpers.h"

#include <memory>

using namespace testing;
using namespace parser;

namespace {

TEST(LR1Parser, parsesTestProgram) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());

    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
    ParsingTable parsingTable { &grammar };

    LR1Parser parser { parsingTable };
    scanner::LexicalSession session;
    scanner::LexFileScannerReader scannerReader;
    auto scanner = std::make_unique<scanner::Scanner>(
            getTestResourcePath("programs/example_prog.c"),
            scannerReader.fromConfiguration(configuration.getLexPath()), session);
    auto builder = ast::AbstractSyntaxTreeBuilder::create(
            &grammar, session, configuration.gnuExtensions());
    ASSERT_NO_THROW(parser.parse(*scanner, *builder));
}

}
