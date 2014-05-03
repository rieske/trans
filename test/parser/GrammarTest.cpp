#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/Grammar.h"

#include <stdexcept>

TEST(Grammar, readsGrammarConfiguration) {
	ASSERT_NO_THROW(Grammar grammar("resources/configuration/grammar.bnf"));
}

TEST(Grammar, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
	ASSERT_THROW(Grammar grammar("resources/configuration/nonexistentFile.abc"), std::invalid_argument);
}
