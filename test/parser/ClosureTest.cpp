#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileGrammar.h"
#include "parser/Closure.h"

namespace {

using namespace parser;
using namespace testing;

TEST(Closure, computesClosure) {
    BNFFileGrammar grammar { "test/grammars/closure_grammar.bnf" };
    FirstTable firstTable { grammar.getNonterminals() };
    Closure closure { firstTable };

    std::vector<LR1Item> items { LR1Item { grammar.getStartSymbol(), 0, { grammar.getEndSymbol() } } };
    closure(items);

    EXPECT_THAT(items, SizeIs(6));
}

}
