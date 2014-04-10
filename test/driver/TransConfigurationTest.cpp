#include "driver/TransConfiguration.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;

TEST(TransConfiguration, createsDefaultTransConfiguration) {
	char executable[] = "trans";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, sourceFileName };

	TransConfiguration configuration(2, argv);

	ASSERT_THAT(configuration.getCustomGrammarFileName(), IsEmpty());
	ASSERT_THAT(configuration.isScannerLoggingEnabled(), Eq(false));
	ASSERT_THAT(configuration.isParserLoggingEnabled(), Eq(false));
	ASSERT_THAT(configuration.getSourceFileNames().size(), Eq(1));
	ASSERT_THAT(*configuration.getSourceFileNames().begin(), StrEq("test.src"));
}

TEST(TransConfiguration, handlesMultipleSourceFiles) {
	char executable[] = "trans";
	char sourceFileName1[] = "test1.src";
	char sourceFileName2[] = "test2.src";
	char sourceFileName3[] = "test3.src";
	char *argv[] = { executable, sourceFileName1, sourceFileName2, sourceFileName3 };

	TransConfiguration configuration(4, argv);

	ASSERT_THAT(configuration.getSourceFileNames().size(), Eq(3));
	std::vector<std::string>::const_iterator sourceFileNamesIterator = configuration.getSourceFileNames().begin();
	ASSERT_THAT(*sourceFileNamesIterator, StrEq("test1.src"));
	ASSERT_THAT(*++sourceFileNamesIterator, StrEq("test2.src"));
	ASSERT_THAT(*++sourceFileNamesIterator, StrEq("test3.src"));
}

TEST(TransConfiguration, terminatesForIllegalArguments) {
	ASSERT_EXIT(TransConfiguration configuration(0, 0), ExitedWithCode(EXIT_FAILURE), "");
}

TEST(TransConfiguration, terminatesIfNoSourceFilesSpecified) {
	char executable[] = "trans";
	char *argv[] = { executable };

	ASSERT_EXIT(TransConfiguration configuration(1, argv), ExitedWithCode(EXIT_FAILURE), "");
}

TEST(TransConfiguration, exitsSuccessfullyWhenHelpRequested) {
	char executable[] = "trans";
	char helpArg[] = "-h";
	char *argv[] = { executable, helpArg };

	ASSERT_EXIT(TransConfiguration configuration(2, argv), ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(TransConfiguration, exitsForIncorrectArguments) {
	char executable[] = "trans";
	char loggingArg[] = "-l";
	char *argv[] = { executable, loggingArg };

	ASSERT_EXIT(TransConfiguration configuration(2, argv), ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(TransConfiguration, setsCustomGrammarFileName) {
	char executable[] = "trans";
	char grammarArg[] = "-ggrammar.bnf";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, grammarArg, sourceFileName };

	TransConfiguration configuration(3, argv);

	ASSERT_THAT(configuration.getCustomGrammarFileName(), StrEq("grammar.bnf"));
	ASSERT_THAT(*configuration.getSourceFileNames().begin(), StrEq("test.src"));
}

TEST(TransConfiguration, setsScannerLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-ls";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, loggingArg, sourceFileName };

	TransConfiguration configuration(3, argv);

	ASSERT_TRUE(configuration.isScannerLoggingEnabled());
	ASSERT_FALSE(configuration.isParserLoggingEnabled());
	ASSERT_THAT(*configuration.getSourceFileNames().begin(), StrEq("test.src"));
}

TEST(TransConfiguration, setsParserLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-lp";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, loggingArg, sourceFileName };

	TransConfiguration configuration(3, argv);

	ASSERT_TRUE(configuration.isParserLoggingEnabled());
	ASSERT_FALSE(configuration.isScannerLoggingEnabled());
	ASSERT_THAT(*configuration.getSourceFileNames().begin(), StrEq("test.src"));
}

TEST(TransConfiguration, setsParserAndScannerLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-lsp";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, loggingArg, sourceFileName };

	TransConfiguration configuration(3, argv);

	ASSERT_TRUE(configuration.isParserLoggingEnabled());
	ASSERT_TRUE(configuration.isScannerLoggingEnabled());
	ASSERT_THAT(*configuration.getSourceFileNames().begin(), StrEq("test.src"));
}

TEST(TransConfiguration, terminatesGivenInvalidLoggingArgument) {
	char executable[] = "trans";
	char invalidLoggingArg[] = "-lo";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, invalidLoggingArg, sourceFileName };

	ASSERT_EXIT(TransConfiguration configuration(3, argv);, ExitedWithCode(EXIT_FAILURE), "");
}
