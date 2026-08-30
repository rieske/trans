#include "parser/BNFFileReader.h"
#include "parser/GenerateParsingTable.h"
#include "parser/GrammarBuilder.h"
#include "parser/ParsingTable.h"

#include "gtest/gtest.h"

#include "ResourceHelpers.h"
#include "TableAssertions.h"
#include "scanner/Token.h"

using namespace parser;

namespace {

Grammar productGrammar() {
    BNFFileReader reader;
    return reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
}

Grammar expressionGrammar() {
    GrammarBuilder builder;
    builder.defineRule("<expr>", {"<term>", "+", "<expr>"});
    builder.defineRule("<expr>", {"<term>"});
    builder.defineRule("<term>", {"<factor>", "*", "<term>"});
    builder.defineRule("<term>", {"<factor>"});
    builder.defineRule("<factor>", {"(", "<expr>", ")"});
    builder.defineRule("<factor>", {"<operand>"});
    builder.defineRule("<operand>", {"identifier"});
    builder.defineRule("<operand>", {"constant"});
    return builder.build();
}

} // namespace

TEST(ParsingTableProduct, matchesGeneratedTable) {
    Grammar grammar = productGrammar();
    ParsingTable fromGenerated = generateParsingTable(&grammar, AutomatonKind::LALR1);
    ParsingTable fromProduct { &grammar };
    expectTablesMatch(fromGenerated, fromProduct);
}

TEST(ParsingTableProduct, unknownLookaheadIsErrorActionNotThrow) {
    Grammar grammar = productGrammar();
    ParsingTable table { &grammar };

    scanner::Token token { "_Generic", "_Generic", { "t.c", 1 },
            grammar.trySymbolId("_Generic").value_or(99999) };
    Action action;
    ASSERT_NO_THROW(action = table.action(0, token));
    EXPECT_EQ(action.kind(), Action::Kind::Error);
}

TEST(ParsingTableProduct, rejectsGrammarThatIsNotTheProductGrammar) {
    Grammar grammar = expressionGrammar();
    EXPECT_THROW((ParsingTable { &grammar }), std::runtime_error);
}
