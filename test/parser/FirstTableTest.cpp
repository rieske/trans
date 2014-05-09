#include "parser/FirstTable.h"
#include "parser/BNFReader.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;

TEST(FirstTable, computesFirstTableForGrammarRules) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };

	FirstTable firstTable { bnfReader.getRules() };
}
