#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "src/parser/BNFFileGrammar.h"
#include "src/parser/Closure.h"

#include "ResourceHelpers.h"

namespace {

using namespace parser;
using namespace testing;

TEST(Closure, computesClosure) {
    BNFFileGrammar grammar { getTestResourcePath("grammars/closure_grammar.bnf") };
    FirstTable firstTable { grammar };
    Closure closure { firstTable, &grammar };

    std::vector<LR1Item> items { LR1Item { grammar.getRuleByIndex(grammar.ruleCount() - 1), { grammar.getEndSymbol() } } };
    closure(items);

    EXPECT_THAT(items, SizeIs(6));

    std::stringstream sstream;
    sstream << items.at(0) << items.at(1) << items.at(2) << items.at(3) << items.at(4) << items.at(5);
    EXPECT_THAT(sstream.str(), Eq("[ <__start__> -> . <S> , '$end$' ]\n"
            "[ <S> -> . <L> = <R> , '$end$' ]\n"
            "[ <S> -> . <R> , '$end$' ]\n"
            "[ <L> -> . * <R> , '$end$' = ]\n"
            "[ <L> -> . id , '$end$' = ]\n"
            "[ <R> -> . <L> , '$end$' ]\n"));
}

}
