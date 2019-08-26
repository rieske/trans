#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "src/scanner/LexFileFiniteAutomaton.h"

#include "ResourceHelpers.h"


TEST(LexFileFiniteAutomaton, readsAutomatonConfiguration) {
	ASSERT_NO_THROW(LexFileFiniteAutomaton factory(getResourcePath("configuration/scanner.lex")));
}

TEST(LexFileFiniteAutomaton, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
	ASSERT_THROW(LexFileFiniteAutomaton factory(getResourcePath("configuration/nonexistentFile.abc")), std::invalid_argument);
}
