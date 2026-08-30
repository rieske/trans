#include "parser/CompileParsingTable.h"
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

Grammar tinyGrammar() {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    return builder.build();
}

ParsingTable emptyTable(const Grammar& grammar) {
    LookaheadActionTable actions;
    actions.setStateCount(1);
    return compileParsingTable(actions, {}, grammar);
}

scanner::Token tokenFor(const Grammar& grammar, int symbolId) {
    return { grammar.getSymbolById(symbolId), grammar.getSymbolById(symbolId), { "t.c", 1 }, symbolId };
}

TEST(ParsingTable, missingExplicitCellUsesStoredErrorCandidates) {
    Grammar grammar = tinyGrammar();
    LookaheadActionTable actions;
    actions.setStateCount(1);
    actions.setErrorCandidates(0, { grammar.getTerminalIDs().front() });
    ParsingTable table = compileParsingTable(actions, {}, grammar);

    const Action err = table.action(0, tokenFor(grammar, grammar.getEndSymbol()));
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.toString(), Eq("e 0 " + std::to_string(grammar.getTerminalIDs().front())));
}

TEST(ParsingTable, missingExplicitCellWithoutCandidatesIsBareError) {
    Grammar grammar = tinyGrammar();
    ParsingTable table = emptyTable(grammar);

    const Action err = table.action(0, tokenFor(grammar, grammar.getEndSymbol()));
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.toString(), Eq("e 0"));
}

TEST(ParsingTable, unknownLookaheadIsError) {
    Grammar grammar = tinyGrammar();
    ParsingTable table = emptyTable(grammar);

    const scanner::Token unknown { "_Generic", "_Generic", { "t.c", 1 }, 99999 };
    const Action err = table.action(0, unknown);
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.toString(), HasSubstr("e "));
}

TEST(ParsingTable, unstampedLookaheadIsProgrammingError) {
    Grammar grammar = tinyGrammar();
    ParsingTable table = emptyTable(grammar);

    EXPECT_THROW(table.action(0, { "a", "a", { "t.c", 1 } }), std::logic_error);
}

} // namespace
