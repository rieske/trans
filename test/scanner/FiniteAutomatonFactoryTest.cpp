#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/FiniteAutomatonFactory.h"

#include <stdexcept>

TEST(FiniteAutomatonFactory, readsAutomatonConfiguration) {
	FiniteAutomatonFactory factory("resources/configuration/scanner.lex");

}

TEST(FiniteAutomatonFactory, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
	ASSERT_THROW(FiniteAutomatonFactory factory("resources/configuration/nonexistentFile.abc"), std::invalid_argument);
}
