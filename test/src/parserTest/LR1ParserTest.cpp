#include "gtest/gtest.h"

#include "scanner/LexicalSession.h"
#include "gmock/gmock.h"

#include "parser/LR1Parser.h"
#include "parser/BNFFileReader.h"
#include "parser/Grammar.h"
#include "parser/ParsingTable.h"
#include "util/LogManager.h"
#include "ast/AbstractSyntaxTree.h"
#include "ast/SyntaxTreeBuilder.h"
#include "driver/Configuration.h"
#include "scanner/LexFileScannerReader.h"
#include "scanner/Scanner.h"

#include "ResourceHelpers.h"
#include "util/Diagnostic.h"

#include <memory>
#include <sstream>

using namespace testing;
using namespace parser;

namespace {

bool parseSource(const std::string& name, const std::string& source) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());

    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
    ParsingTable parsingTable { &grammar };

    LR1Parser parser { parsingTable };
    scanner::LexicalSession session;
    scanner::LexFileScannerReader scannerReader;
    const std::string path = writeTempSource(name, source);
    auto scanner = std::make_unique<scanner::Scanner>(
            path, scannerReader.fromConfiguration(configuration.getLexPath()), session);
    auto builder = ast::SyntaxTreeBuilder::create(
            &grammar, session, configuration.gnuExtensions());
    std::ostringstream logged;
    diag::Sink sink { logged };
    builder->setSink(&sink);
    return parser.parse(*scanner, *builder) && !builder->hasError();
}

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
    auto builder = ast::SyntaxTreeBuilder::create(
            &grammar, session, configuration.gnuExtensions());
    ASSERT_TRUE(parser.parse(*scanner, *builder));
    std::unique_ptr<ast::AbstractSyntaxTree> tree = builder->buildTree();
    ASSERT_TRUE(tree != nullptr);
}

TEST(LR1Parser, unexpectedTokenSetsErrorAndWritesSink) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());

    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
    ParsingTable parsingTable { &grammar };

    LR1Parser parser { parsingTable };
    scanner::LexicalSession session;
    scanner::LexFileScannerReader scannerReader;
    const std::string path = writeTempSource("lr1_unexpected_token", "}\n");
    auto scanner = std::make_unique<scanner::Scanner>(
            path, scannerReader.fromConfiguration(configuration.getLexPath()), session);
    auto builder = ast::SyntaxTreeBuilder::create(
            &grammar, session, configuration.gnuExtensions());
    std::ostringstream logged;
    diag::Sink sink { logged };
    builder->setSink(&sink);

    EXPECT_FALSE(parser.parse(*scanner, *builder));
    EXPECT_TRUE(builder->hasError());
    EXPECT_TRUE(sink.hasErrors());
    EXPECT_THAT(logged.str(), HasSubstr("unexpected token"));
}

TEST(LR1Parser, parsesKeywordHeavyTranslationUnit) {
    EXPECT_TRUE(parseSource("lr1_keywords.c",
            "int f(void) { int x; x = 1; if (x) return x; return 0; }\n"));
}

TEST(LR1Parser, parsesGnuStatementExpression) {
    EXPECT_TRUE(parseSource("lr1_stmt_expr.c",
            "int f(void) { return ({ int y; y = 1; y; }); }\n"));
}

TEST(LR1Parser, parsesGnuInt128) {
    EXPECT_TRUE(parseSource("lr1_int128.c",
            "__int128 f(__int128 x) { return x; }\n"));
}

}
