#include "parser/BNFFileGrammar.h"
#include "parser/Grammar.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;
using namespace parser;


TEST(BNFFileGrammar, readsBNFGrammarConfiguration) {
	BNFFileGrammar grammar { "resources/configuration/grammar.bnf" };

	EXPECT_THAT(grammar.ruleCount(), Eq(243));

	EXPECT_THAT(grammar.getTerminals(), SizeIs(88));
	EXPECT_THAT(grammar.getNonterminals(), SizeIs(68));
}

TEST(BNFFileGrammar, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
	ASSERT_THROW(BNFFileGrammar { "resources/configuration/nonexistentFile.abc" }, std::invalid_argument);
}
