#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/Action.h"
#include "parser/GrammarBuilder.h"

#include <memory>
#include <vector>

namespace {

using namespace parser;
using testing::Eq;

TEST(Action, equalsComparesKindsAndPayloads) {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    const Production& production = grammar.getRuleById(0);

    Action shift1 = Action::shift(1);
    Action shift2 = Action::shift(2);
    Action reduce = Action::reduce(production);
    Action accept = Action::accept();
    auto cands = std::make_shared<const std::vector<int>>(std::vector<int>{ 1, 2 });
    Action error = Action::error(0, cands);
    Action errorSame = Action::error(0, cands);
    Action errorOther = Action::error(0,
            std::make_shared<const std::vector<int>>(std::vector<int>{ 3 }));

    EXPECT_TRUE(shift1.equals(Action::shift(1)));
    EXPECT_FALSE(shift1.equals(shift2));
    EXPECT_FALSE(shift1.equals(accept));
    EXPECT_TRUE(accept.equals(Action::accept()));
    EXPECT_TRUE(reduce.equals(Action::reduce(production)));
    EXPECT_TRUE(error.equals(errorSame));
    EXPECT_FALSE(error.equals(errorOther));
    EXPECT_FALSE(reduce.equals(error));
    EXPECT_TRUE(reduce.isCorrective());
    EXPECT_FALSE(shift1.isCorrective());
    EXPECT_EQ(shift1.shiftState(), 1u);
    EXPECT_EQ(reduce.productionId(), production.getId());
}

TEST(Action, toStringReduceAndError) {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    Grammar grammar = builder.build();
    const Production& production = grammar.getRuleById(0);

    Action reduce = Action::reduce(production);
    EXPECT_THAT(reduce.toString(), Eq("r " + std::to_string(production.getId())));
    EXPECT_THAT(Action::shift(42).toString(), Eq("s 42"));
    EXPECT_THAT(Action::accept().toString(), Eq("a"));

    auto cands = std::make_shared<const std::vector<int>>(std::vector<int>{ grammar.getEndSymbol() });
    Action error = Action::error(0, cands);
    EXPECT_THAT(error.toString(), Eq("e 0 " + std::to_string(grammar.getEndSymbol())));
}

} // namespace
