#include "parser/BNFReader.h"
#include "parser/Grammar.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;

TEST(BNFReader, readsBNFGrammarConfiguration) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };

	Grammar grammar = bnfReader.getGrammar();
	ASSERT_THAT(grammar.terminals, SizeIs(58));
	ASSERT_THAT(grammar.nonterminals, SizeIs(44));
	ASSERT_THAT(bnfReader.getIdToTerminalMappingTable(), SizeIs(grammar.terminals.size()));
}

TEST(BNFReader, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
	ASSERT_THROW(BNFReader { "resources/configuration/nonexistentFile.abc" }, std::invalid_argument);
}
