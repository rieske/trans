#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/Action.h"
#include "parser/GrammarBuilder.h"
#include "parser/ParsingTable.h"
#include "parser/ParseTreeBuilder.h"
#include "parser/TokenStream.h"

#include <memory>
#include <stack>
#include <stdexcept>
#include <vector>

namespace {

using namespace parser;
using testing::Eq;

TEST(Action, equalsComparesKindsAndPayloads) {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    ParsingTable table { &grammar };
    const Production& production = grammar.getRuleById(0);

    Action shift1 = Action::shift(1);
    Action shift2 = Action::shift(2);
    Action reduce = Action::reduce(production, &table);
    Action accept = Action::accept();
    auto cands = std::make_shared<const std::vector<int>>(std::vector<int>{ 1, 2 });
    Action error = Action::error(0, cands, &grammar);
    Action errorSame = Action::error(0, cands, &grammar);
    Action errorOther = Action::error(0,
            std::make_shared<const std::vector<int>>(std::vector<int>{ 3 }),
            &grammar);

    EXPECT_TRUE(shift1.equals(Action::shift(1)));
    EXPECT_FALSE(shift1.equals(shift2));
    EXPECT_FALSE(shift1.equals(accept));
    EXPECT_TRUE(accept.equals(Action::accept()));
    EXPECT_TRUE(reduce.equals(Action::reduce(production, &table)));
    EXPECT_TRUE(error.equals(errorSame));
    EXPECT_FALSE(error.equals(errorOther));
    EXPECT_FALSE(reduce.equals(error));
    EXPECT_TRUE(reduce.isCorrective());
    EXPECT_FALSE(shift1.isCorrective());
}

TEST(Action, serializesAndDeserializesReduceAndError) {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    ParsingTable table { &grammar };
    const Production& production = grammar.getRuleById(0);

    Action reduce = Action::reduce(production, &table);
    EXPECT_THAT(reduce.serialize(), Eq("r " + std::to_string(production.getId())));
    Action reduceRoundTrip = Action::deserialize(reduce.serialize(), table, grammar);
    EXPECT_TRUE(reduce.equals(reduceRoundTrip));

    auto cands = std::make_shared<const std::vector<int>>(std::vector<int>{ grammar.getEndSymbol() });
    Action error = Action::error(0, cands, &grammar);
    EXPECT_THAT(error.serialize(), Eq("e 0 " + std::to_string(grammar.getEndSymbol())));
    Action errorRoundTrip = Action::deserialize(error.serialize(), table, grammar);
    EXPECT_TRUE(error.equals(errorRoundTrip));
}

TEST(Action, deserializeRejectsUnknownType) {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    ParsingTable table { &grammar };
    EXPECT_THROW(Action::deserialize("x 1", table, grammar), std::runtime_error);
}

TEST(Action, errorParseReportsAndStops) {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    auto cands = std::make_shared<const std::vector<int>>(std::vector<int>{ grammar.getTerminalIDs().front() });
    Action error = Action::error(0, cands, &grammar);

    std::stack<parse_state> stack;
    TokenStream tokens { []() { return scanner::Token{ "a", "a", { "", 1 } }; } };
    ParseTreeBuilder treeBuilder { "test", &grammar };
    EXPECT_TRUE(error.parse(stack, tokens, treeBuilder));
}

} // namespace
