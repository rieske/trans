#include "parser/BNFReader.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;

TEST(BNFReader, readsBNFGrammarConfiguration) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };

	ASSERT_THAT(bnfReader.getTerminals(), SizeIs(58));
	ASSERT_THAT(bnfReader.getNonterminals(), SizeIs(44));
	ASSERT_THAT(bnfReader.getRules(), SizeIs(126));
	ASSERT_THAT(bnfReader.getIdToTerminalMappingTable(), SizeIs(bnfReader.getTerminals().size()));
}

TEST(BNFReader, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
	ASSERT_THROW(BNFReader { "resources/configuration/nonexistentFile.abc" }, std::invalid_argument);
}
