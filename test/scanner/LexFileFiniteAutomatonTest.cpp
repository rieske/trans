#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/LexFileFiniteAutomaton.h"

#include <stdexcept>

TEST(LexFileFiniteAutomaton, readsAutomatonConfiguration) {
	ASSERT_NO_THROW(LexFileFiniteAutomaton factory("resources/configuration/scanner.lex"));
}

TEST(LexFileFiniteAutomaton, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
	ASSERT_THROW(LexFileFiniteAutomaton factory("resources/configuration/nonexistentFile.abc"), std::invalid_argument);
}
