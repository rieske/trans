#include "parser/BNFReader.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

TEST(BNFReader, readsBNFGrammarConfiguration) {
	ASSERT_NO_THROW(BNFReader { "resources/configuration/grammar.bnf" });
}

TEST(BNFReader, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
	ASSERT_THROW(BNFReader { "resources/configuration/nonexistentFile.abc" }, std::invalid_argument);
}
