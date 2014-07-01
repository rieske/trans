#include "parser/CanonicalCollection.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileGrammar.h"
#include "parser/Grammar.h"

using namespace testing;
using std::vector;
using std::shared_ptr;

TEST(CanonicalCollection, computesCanonicalCollectionForTheGrammar) {
	BNFFileGrammar grammar { "resources/configuration/grammar.bnf" };

	FirstTable firstTable { grammar.getNonterminals() };
	CanonicalCollection canonicalCollection { firstTable, grammar };
	ASSERT_THAT(canonicalCollection.stateCount(), Eq(809));
}
