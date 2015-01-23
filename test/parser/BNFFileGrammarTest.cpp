#include "parser/BNFFileGrammar.h"
#include "parser/Grammar.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;
using namespace parser;


TEST(BNFFileGrammar, readsBNFGrammarConfiguration) {
	BNFFileGrammar grammar { "resources/grammars/grammar_original.bnf" };

	ASSERT_THAT(grammar.getTerminals(), SizeIs(59));
	ASSERT_THAT(grammar.getNonterminals(), SizeIs(44));
}

TEST(BNFFileGrammar, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
	ASSERT_THROW(BNFFileGrammar { "resources/configuration/nonexistentFile.abc" }, std::invalid_argument);
}
