#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/Action.h"
#include "parser/GrammarBuilder.h"
#include "parser/LookaheadActionTable.h"

#include <memory>
#include <stdexcept>
#include <vector>

namespace {

using namespace parser;
using testing::ElementsAre;
using testing::Eq;
using testing::IsNull;

TEST(LookaheadActionTable, reportsMissingExplicitAction) {
    LookaheadActionTable table;
    EXPECT_THAT(table.findAction(0, 1), IsNull());
    table.addAction(0, 1, Action::shift(3));
    ASSERT_NE(table.findAction(0, 1), nullptr);
    EXPECT_THAT(table.findAction(0, 1)->toString(), Eq("s 3"));
    EXPECT_FALSE(table.hasCorrectiveAction(0, 1));
    EXPECT_THAT(table.findAction(0, 2), IsNull());
    EXPECT_THAT(table.findAction(1, 1), IsNull());
}

TEST(LookaheadActionTable, ignoresDuplicateCompatibleAction) {
    LookaheadActionTable table;
    table.addAction(0, 7, Action::shift(4));
    EXPECT_NO_THROW(table.addAction(0, 7, Action::shift(4)));
    ASSERT_NE(table.findAction(0, 7), nullptr);
    EXPECT_THAT(table.findAction(0, 7)->toString(), Eq("s 4"));
}

TEST(LookaheadActionTable, throwsOnConflictingAction) {
    LookaheadActionTable table;
    table.addAction(0, 7, Action::shift(4));
    EXPECT_THROW(table.addAction(0, 7, Action::shift(5)), std::runtime_error);
    EXPECT_THROW(table.addAction(0, 7, Action::accept()), std::runtime_error);
}

TEST(LookaheadActionTable, addActionRejectsErrorCells) {
    LookaheadActionTable table;
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    auto candidates = std::make_shared<const std::vector<int>>(std::vector<int>{ 1 });
    EXPECT_THROW(table.addAction(0, 1, Action::error(0, candidates, &grammar)), std::runtime_error);
}

TEST(LookaheadActionTable, storesErrorCandidatesWithoutSynthesizingActions) {
    LookaheadActionTable table;
    table.setErrorCandidates(0, { 4, 7 });
    EXPECT_THAT(table.size(), Eq(1u));
    const auto rows = table.errorRows();
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].state, 0u);
    EXPECT_THAT(rows[0].candidates, ElementsAre(4, 7));
    EXPECT_THAT(table.findAction(0, 4), IsNull());
}

TEST(LookaheadActionTable, setErrorCandidatesGrowsStateCount) {
    LookaheadActionTable table;
    table.setErrorCandidates(3, { 1 });
    EXPECT_THAT(table.size(), Eq(4u));
}

TEST(LookaheadActionTable, emptyErrorCandidatesEraseTheRow) {
    LookaheadActionTable table;
    table.setErrorCandidates(0, { 1 });
    table.setErrorCandidates(0, {});
    EXPECT_TRUE(table.errorRows().empty());
}

} // namespace
