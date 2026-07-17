#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileReader.h"
#include "parser/CanonicalCollection.h"
#include "parser/FirstTable.h"
#include "scanner/Token.h"

#include "ResourceHelpers.h"

namespace {

using namespace parser;
using namespace testing;

// State 0 of the canonical collection is the closure of the augmented start item.
// Items are ordered by coreKey (production id, then dot position).
void expectStartClosure(const std::vector<LR1Item>& items, const Grammar& grammar) {
    EXPECT_THAT(items, SizeIs(6));
    EXPECT_THAT(items.at(0).str(grammar), Eq("[ <L> -> . * <R> , = " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(1).str(grammar), Eq("[ <L> -> . id , = " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(2).str(grammar), Eq("[ <R> -> . <L> , " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(3).str(grammar), Eq("[ <S> -> . <L> = <R> , " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(4).str(grammar), Eq("[ <S> -> . <R> , " + scanner::Token::END + " ]\n"));
    EXPECT_THAT(items.at(5).str(grammar), Eq("[ <__start__> -> . <S> , " + scanner::Token::END + " ]\n"));
}

TEST(Closure, startStateClosure_LALR1) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/closure_grammar.bnf"));
    FirstTable firstTable { grammar };
    CanonicalCollection collection { firstTable, grammar, AutomatonKind::LALR1 };
    expectStartClosure(collection.setOfItemsAtState(0), grammar);
}

TEST(Closure, startStateClosure_LR1) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/closure_grammar.bnf"));
    FirstTable firstTable { grammar };
    CanonicalCollection collection { firstTable, grammar, AutomatonKind::LR1 };
    expectStartClosure(collection.setOfItemsAtState(0), grammar);
}

} // namespace
