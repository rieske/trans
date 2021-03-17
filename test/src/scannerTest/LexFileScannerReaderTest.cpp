#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"
#include "scanner/LexFileScannerReader.h"

using namespace scanner;

TEST(LexFileScannerReader, readsAutomatonConfiguration) {
    LexFileScannerReader reader;
	ASSERT_NO_THROW(reader.fromConfiguration(getResourcePath("configuration/scanner.lex")));
}

TEST(LexFileScannerReader, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
    LexFileScannerReader reader;
	ASSERT_THROW(reader.fromConfiguration(getResourcePath("configuration/nonexistentFile.abc")), std::invalid_argument);
}

