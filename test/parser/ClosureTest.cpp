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

    std::stringstream sstream;
    sstream << items.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <__start__> -> . <S> , '$end$' ]\n"));

    sstream.str("");
    sstream << items.at(1);
    EXPECT_THAT(sstream.str(), Eq("[ <S> -> . <L> = <R> , '$end$' ]\n"));

    sstream.str("");
    sstream << items.at(2);
    EXPECT_THAT(sstream.str(), Eq("[ <S> -> . <R> , '$end$' ]\n"));

    sstream.str("");
    sstream << items.at(3);
    EXPECT_THAT(sstream.str(), Eq("[ <L> -> . * <R> , = '$end$' ]\n"));

    sstream.str("");
    sstream << items.at(4);
    EXPECT_THAT(sstream.str(), Eq("[ <L> -> . id , = '$end$' ]\n"));

    sstream.str("");
    sstream << items.at(5);
    EXPECT_THAT(sstream.str(), Eq("[ <R> -> . <L> , '$end$' ]\n"));
}

}
