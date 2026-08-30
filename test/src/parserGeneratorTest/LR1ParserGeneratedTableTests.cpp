#include "gtest/gtest.h"

#include "scanner/LexicalSession.h"
#include "gmock/gmock.h"

#include "parser/BNFFileReader.h"
#include "parser/LR1Parser.h"
#include "parser/GenerateParsingTable.h"
#include "parser/ParsingTable.h"
#include "parser/CanonicalCollection.h"
#include "parser/FirstTable.h"
#include "driver/Configuration.h"
#include "driver/CompilerComponentsFactory.h"
#include "parser/SyntaxTreeBuilder.h"

#include "ResourceHelpers.h"

#include <memory>

using namespace testing;
using namespace parser;

namespace {

constexpr const char* kProductGrammar = "configuration/grammar.bnf";

// Generate a parsing table for the product grammar and parse the example program.
void generateAndParseExample(AutomatonKind kind) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());

    CompilerComponentsFactory factory { configuration };
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath(kProductGrammar));
    ParsingTable table = generateParsingTable(&grammar, kind);
    LR1Parser parser { table };
    scanner::LexicalSession session;
    auto syntaxTreeBuilder = factory.makeSyntaxTreeBuilder(&grammar, session);
    ASSERT_NO_THROW(
            parser.parse(*factory.makeScannerForSourceFile(
                    getTestResourcePath("programs/example_prog.c"), session),
                    *syntaxTreeBuilder));
}

// Full generate + parse path for both automaton kinds on the product grammar.
TEST(LR1Parser, generatesAndParsesProductGrammar_LR1) {
    generateAndParseExample(AutomatonKind::LR1);
}

TEST(LR1Parser, generatesAndParsesProductGrammar_LALR1) {
    generateAndParseExample(AutomatonKind::LALR1);
}

// Structural relationship: LR(1) has at least as many states as LALR(1).
TEST(LR1Parser, lr1HasAtLeastAsManyStatesAsLalrOnProductGrammar) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath(kProductGrammar));
    FirstTable first { grammar };
    CanonicalCollection lalr { first, grammar, AutomatonKind::LALR1 };
    CanonicalCollection lr1 { first, grammar, AutomatonKind::LR1 };
    EXPECT_GE(lr1.stateCount(), lalr.stateCount());
    EXPECT_GE(lalr.stateCount(), 1u);
    // Known sizes for the product grammar (guards accidental collapse/explosion).
    EXPECT_EQ(lalr.stateCount(), 479u);
    EXPECT_EQ(lr1.stateCount(), 2540u);
}

} // namespace
