#include "parser/CanonicalCollection.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFReader.h"
#include "parser/Grammar.h"

using namespace testing;
using std::vector;
using std::shared_ptr;

TEST(CanonicalCollection, computesCanonicalCollectionForTheGrammar) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };

	Grammar grammar { bnfReader.getTerminals(), bnfReader.getNonterminals() };
	FirstTable firstTable { grammar.nonterminals };
	CanonicalCollection canonicalCollection { firstTable };
	ASSERT_THAT(canonicalCollection.computeForGrammar(grammar), SizeIs(809));
}
