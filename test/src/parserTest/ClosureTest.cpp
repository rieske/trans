#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileReader.h"
#include "parser/Closure.h"
#include "scanner/Token.h"

#include "ResourceHelpers.h"

namespace {

using namespace parser;
using namespace testing;

TEST(Closure, computesClosure) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/closure_grammar.bnf"));
    FirstTable firstTable { grammar };
    Closure closure { firstTable, &grammar };

    std::vector<LR1Item> items { LR1Item { grammar.getTopRule(), { grammar.getEndSymbol() } } };
    closure(items);

    EXPECT_THAT(items, SizeIs(6));
    EXPECT_THAT(items.at(0).str(grammar), Eq("[ <__start__> -> . <S> , " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(1).str(grammar), Eq("[ <S> -> . <L> = <R> , " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(2).str(grammar), Eq("[ <S> -> . <R> , " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(3).str(grammar), Eq("[ <L> -> . * <R> , = " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(4).str(grammar), Eq("[ <L> -> . id , = " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(5).str(grammar), Eq("[ <R> -> . <L> , " + scanner::Token::END + " ]\n"));
}

}
