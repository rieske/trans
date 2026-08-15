#include "parser/GrammarBuilder.h"
#include "parser/LookaheadActionTable.h"
#include "parser/ParsingTable.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/Token.h"

#include <stdexcept>

namespace {

using namespace parser;
using testing::Eq;
using testing::HasSubstr;

class MutableParsingTable: public ParsingTable {
public:
    explicit MutableParsingTable(const Grammar* grammar) :
            ParsingTable(grammar) {
    }

    LookaheadActionTable& cells() {
        return lookaheadActionTable;
    }
};

Grammar tinyGrammar() {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    return builder.build();
}

scanner::Token tokenFor(const Grammar& grammar, int symbolId) {
    return { grammar.getSymbolById(symbolId), grammar.getSymbolById(symbolId), { "t.c", 1 }, symbolId };
}

TEST(ParsingTable, missingExplicitCellUsesStoredErrorCandidates) {
    Grammar grammar = tinyGrammar();
    MutableParsingTable table { &grammar };
    table.cells().setStateCount(1);
    table.cells().setErrorCandidates(0, { grammar.getTerminalIDs().front() });

    const Action err = table.action(0, tokenFor(grammar, grammar.getEndSymbol()));
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.serialize(), Eq("e 0 " + std::to_string(grammar.getTerminalIDs().front())));
}

TEST(ParsingTable, missingExplicitCellWithoutCandidatesIsBareError) {
    Grammar grammar = tinyGrammar();
    MutableParsingTable table { &grammar };
    table.cells().setStateCount(1);

    const Action err = table.action(0, tokenFor(grammar, grammar.getEndSymbol()));
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.serialize(), Eq("e 0"));
}

TEST(ParsingTable, unknownLookaheadIsError) {
    Grammar grammar = tinyGrammar();
    MutableParsingTable table { &grammar };

    const scanner::Token unknown { "_Generic", "_Generic", { "t.c", 1 }, 99999 };
    const Action err = table.action(0, unknown);
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.serialize(), HasSubstr("e "));
}

TEST(ParsingTable, unstampedLookaheadIsProgrammingError) {
    Grammar grammar = tinyGrammar();
    MutableParsingTable table { &grammar };

    EXPECT_THROW(table.action(0, { "a", "a", { "t.c", 1 } }), std::logic_error);
}

} // namespace
