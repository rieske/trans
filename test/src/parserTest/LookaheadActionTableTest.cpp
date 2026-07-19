#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/Action.h"
#include "parser/GrammarBuilder.h"
#include "parser/LookaheadActionTable.h"
#include "parser/ParsingTable.h"

#include <memory>
#include <stdexcept>
#include <vector>

namespace {

using namespace parser;
using testing::Eq;

TEST(LookaheadActionTable, reportsMissingExplicitAction) {
    LookaheadActionTable table;
    EXPECT_THROW(table.action(0, 1), std::out_of_range);
    table.addAction(0, 1, Action::shift(3));
    EXPECT_THAT(table.action(0, 1).serialize(), Eq("s 3"));
    EXPECT_FALSE(table.hasCorrectiveAction(0, 1)); // shift is not corrective
    EXPECT_THROW(table.action(0, 2), std::out_of_range);
    EXPECT_THROW(table.action(1, 1), std::out_of_range);
}

TEST(LookaheadActionTable, ignoresDuplicateCompatibleAction) {
    LookaheadActionTable table;
    table.addAction(0, 7, Action::shift(4));
    EXPECT_NO_THROW(table.addAction(0, 7, Action::shift(4)));
    EXPECT_THAT(table.action(0, 7).serialize(), Eq("s 4"));
}

TEST(LookaheadActionTable, throwsOnConflictingAction) {
    LookaheadActionTable table;
    table.addAction(0, 7, Action::shift(4));
    EXPECT_THROW(table.addAction(0, 7, Action::shift(5)), std::runtime_error);
    EXPECT_THROW(table.addAction(0, 7, Action::accept()), std::runtime_error);
}

TEST(LookaheadActionTable, synthesizesPerStateErrorWhenTerminalMissing) {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    LookaheadActionTable table;
    table.setStateCount(1);
    auto candidates = std::make_shared<const std::vector<int>>(std::vector<int>{ grammar.getTerminalIDs().front() });
    table.setErrorCandidates(0, candidates, &grammar);

    Action err = table.action(0, grammar.getEndSymbol());
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.serialize(), Eq("e 0 " + std::to_string(grammar.getTerminalIDs().front())));
}

TEST(LookaheadActionTable, throwsWhenNoActionAndNoErrorCandidates) {
    LookaheadActionTable table;
    EXPECT_THROW(table.action(0, 1), std::out_of_range);
}

TEST(LookaheadActionTable, setErrorCandidatesGrowsStateCount) {
    LookaheadActionTable table;
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    table.setErrorCandidates(3, nullptr, &grammar);
    EXPECT_THAT(table.size(), Eq(4u));
}

TEST(LookaheadActionTable, emptyErrorCandidatesSynthesizeBareError) {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    LookaheadActionTable table;
    table.setErrorCandidates(0, nullptr, &grammar);

    Action err = table.action(0, grammar.getEndSymbol());
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.serialize(), Eq("e 0"));
}

} // namespace
