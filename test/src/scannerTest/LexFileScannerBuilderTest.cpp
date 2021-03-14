#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"
#include "scanner/LexFileScannerBuilder.h"

using namespace scanner;

TEST(LexFileScannerBuilder, readsAutomatonConfiguration) {
    LexFileScannerBuilder builder;
	ASSERT_NO_THROW(builder.fromConfiguration(getResourcePath("configuration/scanner.lex")));
}

TEST(LexFileScannerBuilder, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
    LexFileScannerBuilder builder;
	ASSERT_THROW(builder.fromConfiguration(getResourcePath("configuration/nonexistentFile.abc")), std::invalid_argument);
}

