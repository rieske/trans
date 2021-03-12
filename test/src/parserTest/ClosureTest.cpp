#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileReader.h"
#include "parser/Closure.h"

#include "ResourceHelpers.h"

namespace {

using namespace parser;
using namespace testing;

TEST(Closure, computesClosure) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/closure_grammar.bnf"));
    FirstTable firstTable { grammar };
    Closure closure { firstTable, &grammar };

    std::vector<LR1Item> items { LR1Item { grammar.getRuleByIndex(grammar.ruleCount() - 1), { grammar.getEndSymbol() } } };
    closure(items);

    EXPECT_THAT(items, SizeIs(6));
    EXPECT_THAT(items.at(0).str(grammar), Eq("[ <__start__> -> . <S> , '$end$' ]\n"));
    EXPECT_THAT(items.at(1).str(grammar), Eq("[ <S> -> . <L> = <R> , '$end$' ]\n"));
    EXPECT_THAT(items.at(2).str(grammar), Eq("[ <S> -> . <R> , '$end$' ]\n"));
    EXPECT_THAT(items.at(3).str(grammar), Eq("[ <L> -> . * <R> , '$end$' = ]\n"));
    EXPECT_THAT(items.at(4).str(grammar), Eq("[ <L> -> . id , '$end$' = ]\n"));
    EXPECT_THAT(items.at(5).str(grammar), Eq("[ <R> -> . <L> , '$end$' ]\n"));
}

}
