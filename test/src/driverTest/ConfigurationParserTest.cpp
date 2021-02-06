#include "driver/ConfigurationParser.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;

TEST(ConfigurationParser, createsDefaultTransConfiguration) {
	char executable[] = "trans";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, sourceFileName };

	ConfigurationParser parser(2, argv);
    Configuration configuration = parser.parseConfiguration();

	ASSERT_THAT(parser.getSourceFileNames(), SizeIs(1));
	ASSERT_THAT(*parser.getSourceFileNames().begin(), StrEq("test.src"));

	ASSERT_THAT(configuration.getGrammarPath(), StrEq("resources/configuration/grammar.bnf"));
	ASSERT_THAT(configuration.isScannerLoggingEnabled(), Eq(false));
	ASSERT_THAT(configuration.isParserLoggingEnabled(), Eq(false));
}

TEST(ConfigurationParser, handlesMultipleSourceFiles) {
	char executable[] = "trans";
	char sourceFileName1[] = "test1.src";
	char sourceFileName2[] = "test2.src";
	char sourceFileName3[] = "test3.src";
	char *argv[] = { executable, sourceFileName1, sourceFileName2, sourceFileName3 };

	ConfigurationParser configuration(4, argv);

	auto sourceFileNames = configuration.getSourceFileNames();
	ASSERT_THAT(sourceFileNames, SizeIs(3));
	auto sourceFileNamesIterator = sourceFileNames.begin();
	ASSERT_THAT(*sourceFileNamesIterator, StrEq("test1.src"));
	ASSERT_THAT(*++sourceFileNamesIterator, StrEq("test2.src"));
	ASSERT_THAT(*++sourceFileNamesIterator, StrEq("test3.src"));
}

// started failing on CI after adding coverage data - temporarily disabling
TEST(ConfigurationParser, terminatesForIllegalArguments) {
	ASSERT_EXIT(ConfigurationParser configuration(0, 0), ExitedWithCode(EXIT_FAILURE), "");
}

TEST(ConfigurationParser, terminatesIfNoSourceFilesSpecified) {
	char executable[] = "trans";
	char *argv[] = { executable };

	ASSERT_EXIT(ConfigurationParser configuration(1, argv), ExitedWithCode(EXIT_FAILURE), "");
}

TEST(ConfigurationParser, exitsSuccessfullyWhenHelpRequested) {
	char executable[] = "trans";
	char helpArg[] = "-h";
	char *argv[] = { executable, helpArg };

	ASSERT_EXIT(ConfigurationParser configuration(2, argv), ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(ConfigurationParser, exitsForIncorrectArguments) {
	char executable[] = "trans";
	char loggingArg[] = "-l";
	char *argv[] = { executable, loggingArg };

	ASSERT_EXIT(ConfigurationParser configuration(2, argv), ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(ConfigurationParser, setsCustomGrammarFileName) {
	char executable[] = "trans";
	char grammarArg[] = "-ggrammar.bnf";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, grammarArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.parseConfiguration();

	ASSERT_THAT(configuration.getGrammarPath(), StrEq("grammar.bnf"));
	ASSERT_THAT(*parser.getSourceFileNames().begin(), StrEq("test.src"));
}

TEST(ConfigurationParser, setsScannerLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-ls";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, loggingArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.parseConfiguration();

	ASSERT_TRUE(configuration.isScannerLoggingEnabled());
	ASSERT_FALSE(configuration.isParserLoggingEnabled());
	ASSERT_THAT(*parser.getSourceFileNames().begin(), StrEq("test.src"));
}

TEST(ConfigurationParser, setsParserLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-lp";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, loggingArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.parseConfiguration();

	ASSERT_TRUE(configuration.isParserLoggingEnabled());
	ASSERT_FALSE(configuration.isScannerLoggingEnabled());
	ASSERT_THAT(*parser.getSourceFileNames().begin(), StrEq("test.src"));
}

TEST(ConfigurationParser, setsParserAndScannerLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-lsp";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, loggingArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.parseConfiguration();

	ASSERT_TRUE(configuration.isParserLoggingEnabled());
	ASSERT_TRUE(configuration.isScannerLoggingEnabled());
	ASSERT_THAT(*parser.getSourceFileNames().begin(), StrEq("test.src"));
}

TEST(ConfigurationParser, terminatesGivenInvalidLoggingArgument) {
	char executable[] = "trans";
	char invalidLoggingArg[] = "-lo";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, invalidLoggingArg, sourceFileName };

	ASSERT_EXIT(ConfigurationParser configuration(3, argv);, ExitedWithCode(EXIT_FAILURE), "");
}
