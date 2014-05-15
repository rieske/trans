#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFReader.h"
#include "parser/Grammar.h"

using namespace testing;

TEST(Grammar, computesCanonicalCollectionForTheGrammar) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };

	Grammar grammar { bnfReader.getTerminals(), bnfReader.getNonterminals(), bnfReader.getRules() };

	ASSERT_THAT(grammar.canonical_collection(), SizeIs(809));
}
