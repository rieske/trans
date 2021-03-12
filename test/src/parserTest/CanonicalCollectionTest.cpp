#include "parser/CanonicalCollection.h"
#include "parser/LR1Strategy.h"
#include "parser/LALR1Strategy.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileGrammar.h"
#include "parser/Grammar.h"

#include "util/LogManager.h"

#include "ResourceHelpers.h"

namespace {

using namespace testing;
using namespace parser;


TEST(LR1CanonicalCollection, computesCanonicalCollectionForTheGrammar) {
    BNFFileGrammar reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/canonical_collection_grammar.bnf"));

    FirstTable firstTable { grammar };
    CanonicalCollection canonicalCollection { firstTable, grammar, LR1Strategy { } };
    EXPECT_THAT(canonicalCollection.stateCount(), Eq(10));

    const auto& state0 = canonicalCollection.setOfItemsAtState(0);
    EXPECT_THAT(state0, SizeIs(4));
    EXPECT_THAT(state0.at(0).str(grammar), Eq("[ <__start__> -> . <S> , '$end$' ]\n"));
    EXPECT_THAT(state0.at(1).str(grammar), Eq("[ <S> -> . <X> <X> , '$end$' ]\n"));
    EXPECT_THAT(state0.at(2).str(grammar), Eq("[ <X> -> . a <X> , a b ]\n"));
    EXPECT_THAT(state0.at(3).str(grammar), Eq("[ <X> -> . b , a b ]\n"));

    const auto& state1 = canonicalCollection.setOfItemsAtState(1);
    EXPECT_THAT(state1, SizeIs(1));
    EXPECT_THAT(state1.at(0).str(grammar), Eq("[ <__start__> -> <S> . , '$end$' ]\n"));

    const auto& state2 = canonicalCollection.setOfItemsAtState(2);
    EXPECT_THAT(state2, SizeIs(3));
    EXPECT_THAT(state2.at(0).str(grammar), Eq("[ <S> -> <X> . <X> , '$end$' ]\n"));
    EXPECT_THAT(state2.at(1).str(grammar), Eq("[ <X> -> . a <X> , '$end$' ]\n"));
    EXPECT_THAT(state2.at(2).str(grammar), Eq("[ <X> -> . b , '$end$' ]\n"));

    const auto& state3 = canonicalCollection.setOfItemsAtState(3);
    EXPECT_THAT(state3, SizeIs(3));
    EXPECT_THAT(state3.at(0).str(grammar), Eq("[ <X> -> a . <X> , a b ]\n"));
    EXPECT_THAT(state3.at(1).str(grammar), Eq("[ <X> -> . a <X> , a b ]\n"));
    EXPECT_THAT(state3.at(2).str(grammar), Eq("[ <X> -> . b , a b ]\n"));

    const auto& state4 = canonicalCollection.setOfItemsAtState(4);
    EXPECT_THAT(state4, SizeIs(1));
    EXPECT_THAT(state4.at(0).str(grammar), Eq("[ <X> -> b . , a b ]\n"));

    const auto& state5 = canonicalCollection.setOfItemsAtState(5);
    EXPECT_THAT(state5, SizeIs(1));
    EXPECT_THAT(state5.at(0).str(grammar), Eq("[ <S> -> <X> <X> . , '$end$' ]\n"));

    const auto& state6 = canonicalCollection.setOfItemsAtState(6);
    EXPECT_THAT(state6, SizeIs(3));
    EXPECT_THAT(state6.at(0).str(grammar), Eq("[ <X> -> a . <X> , '$end$' ]\n"));
    EXPECT_THAT(state6.at(1).str(grammar), Eq("[ <X> -> . a <X> , '$end$' ]\n"));
    EXPECT_THAT(state6.at(2).str(grammar), Eq("[ <X> -> . b , '$end$' ]\n"));

    const auto& state7 = canonicalCollection.setOfItemsAtState(7);
    EXPECT_THAT(state7, SizeIs(1));
    EXPECT_THAT(state7.at(0).str(grammar), Eq("[ <X> -> b . , '$end$' ]\n"));

    const auto& state8 = canonicalCollection.setOfItemsAtState(8);
    EXPECT_THAT(state8, SizeIs(1));
    EXPECT_THAT(state8.at(0).str(grammar), Eq("[ <X> -> a <X> . , a b ]\n"));

    const auto& state9 = canonicalCollection.setOfItemsAtState(9);
    EXPECT_THAT(state9, SizeIs(1));
    EXPECT_THAT(state9.at(0).str(grammar), Eq("[ <X> -> a <X> . , '$end$' ]\n"));

    EXPECT_THAT(canonicalCollection.goTo(0, grammar.symbolId("<S>")), Eq(1));
    EXPECT_THAT(canonicalCollection.goTo(0, grammar.symbolId("<X>")), Eq(2));
    EXPECT_THAT(canonicalCollection.goTo(2, grammar.symbolId("<X>")), Eq(5));
    EXPECT_THAT(canonicalCollection.goTo(3, grammar.symbolId("<X>")), Eq(8));
    EXPECT_THAT(canonicalCollection.goTo(6, grammar.symbolId("<X>")), Eq(9));
}

TEST(LALR1CanonicalCollection, computesCanonicalCollectionForTheGrammar) {
    BNFFileGrammar reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/canonical_collection_grammar.bnf"));

    FirstTable firstTable { grammar };
    CanonicalCollection canonicalCollection { firstTable, grammar, LALR1Strategy { } };
    EXPECT_THAT(canonicalCollection.stateCount(), Eq(7));

    const auto& state0 = canonicalCollection.setOfItemsAtState(0);
    EXPECT_THAT(state0, SizeIs(4));
    EXPECT_THAT(state0.at(0).str(grammar), Eq("[ <__start__> -> . <S> , '$end$' ]\n"));
    EXPECT_THAT(state0.at(1).str(grammar), Eq("[ <S> -> . <X> <X> , '$end$' ]\n"));
    EXPECT_THAT(state0.at(2).str(grammar), Eq("[ <X> -> . a <X> , a b ]\n"));
    EXPECT_THAT(state0.at(3).str(grammar), Eq("[ <X> -> . b , a b ]\n"));

    const auto& state1 = canonicalCollection.setOfItemsAtState(1);
    EXPECT_THAT(state1, SizeIs(1));
    EXPECT_THAT(state1.at(0).str(grammar), Eq("[ <__start__> -> <S> . , '$end$' ]\n"));

    const auto& state2 = canonicalCollection.setOfItemsAtState(2);
    EXPECT_THAT(state2, SizeIs(3));
    EXPECT_THAT(state2.at(0).str(grammar), Eq("[ <S> -> <X> . <X> , '$end$' ]\n"));
    EXPECT_THAT(state2.at(1).str(grammar), Eq("[ <X> -> . a <X> , '$end$' ]\n"));
    EXPECT_THAT(state2.at(2).str(grammar), Eq("[ <X> -> . b , '$end$' ]\n"));

    const auto& state36 = canonicalCollection.setOfItemsAtState(3);
    EXPECT_THAT(state36, SizeIs(3));
    EXPECT_THAT(state36.at(0).str(grammar), Eq("[ <X> -> a . <X> , '$end$' a b ]\n"));
    EXPECT_THAT(state36.at(1).str(grammar), Eq("[ <X> -> . a <X> , '$end$' a b ]\n"));
    EXPECT_THAT(state36.at(2).str(grammar), Eq("[ <X> -> . b , '$end$' a b ]\n"));

    const auto& state47 = canonicalCollection.setOfItemsAtState(4);
    EXPECT_THAT(state47, SizeIs(1));
    EXPECT_THAT(state47.at(0).str(grammar), Eq("[ <X> -> b . , '$end$' a b ]\n"));

    const auto& state5 = canonicalCollection.setOfItemsAtState(5);
    EXPECT_THAT(state5, SizeIs(1));
    EXPECT_THAT(state5.at(0).str(grammar), Eq("[ <S> -> <X> <X> . , '$end$' ]\n"));

    const auto& state89 = canonicalCollection.setOfItemsAtState(6);
    EXPECT_THAT(state89, SizeIs(1));
    EXPECT_THAT(state89.at(0).str(grammar), Eq("[ <X> -> a <X> . , '$end$' a b ]\n"));

    EXPECT_THAT(canonicalCollection.goTo(0, grammar.symbolId("<S>")), Eq(1));
    EXPECT_THAT(canonicalCollection.goTo(0, grammar.symbolId("<X>")), Eq(2));
    EXPECT_THAT(canonicalCollection.goTo(2, grammar.symbolId("<X>")), Eq(5));
    EXPECT_THAT(canonicalCollection.goTo(3, grammar.symbolId("<X>")), Eq(6));
}

} // namespace

