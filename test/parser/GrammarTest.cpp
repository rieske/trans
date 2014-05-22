#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFReader.h"
#include "parser/Grammar.h"

using namespace testing;

TEST(Grammar, computesCanonicalCollectionForTheGrammar) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };

	Grammar grammar { bnfReader.getTerminals(), bnfReader.getNonterminals() };

	ASSERT_THAT(grammar.canonicalCollection(), SizeIs(809));
}
