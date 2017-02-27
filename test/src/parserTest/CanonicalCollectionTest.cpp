#include "src/parser/CanonicalCollection.h"
#include "src/parser/LR1Strategy.h"
#include "src/parser/LALR1Strategy.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "src/parser/BNFFileGrammar.h"
#include "src/parser/Grammar.h"

#include "src/util/LogManager.h"

#include "ResourceHelpers.h"

namespace {

using namespace testing;
using namespace parser;

using std::vector;
using std::shared_ptr;

TEST(LR1CanonicalCollection, computesCanonicalCollectionForTheGrammar) {
    BNFFileGrammar grammar { getTestResourcePath("grammars/canonical_collection_grammar.bnf") };

    FirstTable firstTable { grammar };
    CanonicalCollection canonicalCollection { firstTable, grammar, LR1Strategy { } };
    EXPECT_THAT(canonicalCollection.stateCount(), Eq(10));

    const auto& state0 = canonicalCollection.setOfItemsAtState(0);
    EXPECT_THAT(state0, SizeIs(4));
    std::stringstream sstream;
    sstream << state0.at(0) << state0.at(1) << state0.at(2) << state0.at(3);
    EXPECT_THAT(sstream.str(), Eq("[ <__start__> -> . <S> , '$end$' ]\n"
            "[ <S> -> . <X> <X> , '$end$' ]\n"
            "[ <X> -> . a <X> , a b ]\n"
            "[ <X> -> . b , a b ]\n"));

    const auto& state1 = canonicalCollection.setOfItemsAtState(1);
    EXPECT_THAT(state1, SizeIs(1));
    sstream.str("");
    sstream << state1.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <__start__> -> <S> . , '$end$' ]\n"));

    const auto& state2 = canonicalCollection.setOfItemsAtState(2);
    EXPECT_THAT(state2, SizeIs(3));
    sstream.str("");
    sstream << state2.at(0) << state2.at(1) << state2.at(2);
    EXPECT_THAT(sstream.str(), Eq("[ <S> -> <X> . <X> , '$end$' ]\n"
            "[ <X> -> . a <X> , '$end$' ]\n"
            "[ <X> -> . b , '$end$' ]\n"));

    const auto& state3 = canonicalCollection.setOfItemsAtState(3);
    EXPECT_THAT(state3, SizeIs(3));
    sstream.str("");
    sstream << state3.at(0) << state3.at(1) << state3.at(2);
    EXPECT_THAT(sstream.str(), Eq("[ <X> -> a . <X> , a b ]\n"
            "[ <X> -> . a <X> , a b ]\n"
            "[ <X> -> . b , a b ]\n"));

    const auto& state4 = canonicalCollection.setOfItemsAtState(4);
    EXPECT_THAT(state4, SizeIs(1));
    sstream.str("");
    sstream << state4.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <X> -> b . , a b ]\n"));

    const auto& state5 = canonicalCollection.setOfItemsAtState(5);
    EXPECT_THAT(state5, SizeIs(1));
    sstream.str("");
    sstream << state5.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <S> -> <X> <X> . , '$end$' ]\n"));

    const auto& state6 = canonicalCollection.setOfItemsAtState(6);
    EXPECT_THAT(state6, SizeIs(3));
    sstream.str("");
    sstream << state6.at(0) << state6.at(1) << state6.at(2);
    EXPECT_THAT(sstream.str(), Eq("[ <X> -> a . <X> , '$end$' ]\n"
            "[ <X> -> . a <X> , '$end$' ]\n"
            "[ <X> -> . b , '$end$' ]\n"));

    const auto& state7 = canonicalCollection.setOfItemsAtState(7);
    EXPECT_THAT(state7, SizeIs(1));
    sstream.str("");
    sstream << state7.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <X> -> b . , '$end$' ]\n"));

    const auto& state8 = canonicalCollection.setOfItemsAtState(8);
    EXPECT_THAT(state8, SizeIs(1));
    sstream.str("");
    sstream << state8.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <X> -> a <X> . , a b ]\n"));

    const auto& state9 = canonicalCollection.setOfItemsAtState(9);
    EXPECT_THAT(state9, SizeIs(1));
    sstream.str("");
    sstream << state9.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <X> -> a <X> . , '$end$' ]\n"));

    EXPECT_THAT(canonicalCollection.goTo(0, "<S>"), Eq(1));
    EXPECT_THAT(canonicalCollection.goTo(0, "<X>"), Eq(2));
    EXPECT_THAT(canonicalCollection.goTo(2, "<X>"), Eq(5));
    EXPECT_THAT(canonicalCollection.goTo(3, "<X>"), Eq(8));
    EXPECT_THAT(canonicalCollection.goTo(6, "<X>"), Eq(9));
}

TEST(LALR1CanonicalCollection, computesCanonicalCollectionForTheGrammar) {
    LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });

    BNFFileGrammar grammar { getTestResourcePath("grammars/canonical_collection_grammar.bnf") };

    FirstTable firstTable { grammar };
    CanonicalCollection canonicalCollection { firstTable, grammar, LALR1Strategy { } };
    EXPECT_THAT(canonicalCollection.stateCount(), Eq(7));

    const auto& state0 = canonicalCollection.setOfItemsAtState(0);
    EXPECT_THAT(state0, SizeIs(4));
    std::stringstream sstream;
    sstream << state0.at(0) << state0.at(1) << state0.at(2) << state0.at(3);
    EXPECT_THAT(sstream.str(), Eq("[ <__start__> -> . <S> , '$end$' ]\n"
            "[ <S> -> . <X> <X> , '$end$' ]\n"
            "[ <X> -> . a <X> , a b ]\n"
            "[ <X> -> . b , a b ]\n"));

    const auto& state1 = canonicalCollection.setOfItemsAtState(1);
    EXPECT_THAT(state1, SizeIs(1));
    sstream.str("");
    sstream << state1.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <__start__> -> <S> . , '$end$' ]\n"));

    const auto& state2 = canonicalCollection.setOfItemsAtState(2);
    EXPECT_THAT(state2, SizeIs(3));
    sstream.str("");
    sstream << state2.at(0) << state2.at(1) << state2.at(2);
    EXPECT_THAT(sstream.str(), Eq("[ <S> -> <X> . <X> , '$end$' ]\n"
            "[ <X> -> . a <X> , '$end$' ]\n"
            "[ <X> -> . b , '$end$' ]\n"));

    const auto& state36 = canonicalCollection.setOfItemsAtState(3);
    EXPECT_THAT(state36, SizeIs(3));
    sstream.str("");
    sstream << state36.at(0) << state36.at(1) << state36.at(2);
    EXPECT_THAT(sstream.str(), Eq("[ <X> -> a . <X> , '$end$' a b ]\n"
            "[ <X> -> . a <X> , '$end$' a b ]\n"
            "[ <X> -> . b , '$end$' a b ]\n"));

    const auto& state47 = canonicalCollection.setOfItemsAtState(4);
    EXPECT_THAT(state47, SizeIs(1));
    sstream.str("");
    sstream << state47.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <X> -> b . , '$end$' a b ]\n"));

    const auto& state5 = canonicalCollection.setOfItemsAtState(5);
    EXPECT_THAT(state5, SizeIs(1));
    sstream.str("");
    sstream << state5.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <S> -> <X> <X> . , '$end$' ]\n"));

    const auto& state89 = canonicalCollection.setOfItemsAtState(6);
    EXPECT_THAT(state89, SizeIs(1));
    sstream.str("");
    sstream << state89.at(0);
    EXPECT_THAT(sstream.str(), Eq("[ <X> -> a <X> . , '$end$' a b ]\n"));

    EXPECT_THAT(canonicalCollection.goTo(0, "<S>"), Eq(1));
    EXPECT_THAT(canonicalCollection.goTo(0, "<X>"), Eq(2));
    EXPECT_THAT(canonicalCollection.goTo(2, "<X>"), Eq(5));
    EXPECT_THAT(canonicalCollection.goTo(3, "<X>"), Eq(6));
}

}
