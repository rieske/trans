#include "driver/ConfigurationParser.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;

TEST(ConfigurationParser, createsDefaultTransConfiguration) {
	char executable[] = "trans";
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, sourceFileName };

	ConfigurationParser parser(2, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_THAT(configuration.getSourceFiles(), SizeIs(1));
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.c"));

	ASSERT_THAT(configuration.getGrammarPath(), StrEq("resources/configuration/grammar.bnf"));
	ASSERT_THAT(configuration.isScannerLoggingEnabled(), Eq(false));
	ASSERT_THAT(configuration.isParserLoggingEnabled(), Eq(false));
	ASSERT_THAT(configuration.getAssemblyDialect(), Eq(AssemblyDialect::Intel));
	ASSERT_THAT(configuration.assemblyDialectTag(), StrEq("intel"));
}

TEST(ConfigurationParser, setsIntelAssemblyDialect) {
	char executable[] = "trans";
	char dialectArg[] = "-aintel";
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, dialectArg, sourceFileName };

	ConfigurationParser parser(3, argv);
	Configuration configuration = parser.getConfiguration();

	ASSERT_THAT(configuration.getAssemblyDialect(), Eq(AssemblyDialect::Intel));
	ASSERT_THAT(configuration.assemblyDialectTag(), StrEq("intel"));
}

TEST(ConfigurationParser, setsAtAndTAssemblyDialect) {
	char executable[] = "trans";
	char dialectArg[] = "-aatt";
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, dialectArg, sourceFileName };

	ConfigurationParser parser(3, argv);
	Configuration configuration = parser.getConfiguration();

	ASSERT_THAT(configuration.getAssemblyDialect(), Eq(AssemblyDialect::AtAndT));
	ASSERT_THAT(configuration.assemblyDialectTag(), StrEq("att"));
}

TEST(ConfigurationParser, terminatesGivenUnknownAssemblyDialect) {
	char executable[] = "trans";
	char dialectArg[] = "-agas";
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, dialectArg, sourceFileName };

	ASSERT_EXIT(ConfigurationParser configuration(3, argv);, ExitedWithCode(EXIT_FAILURE), "");
}

TEST(ConfigurationParser, handlesMultipleSourceFiles) {
	char executable[] = "trans";
	char sourceFileName1[] = "test1.c";
	char sourceFileName2[] = "test2.c";
	char sourceFileName3[] = "test3.c";
	char *argv[] = { executable, sourceFileName1, sourceFileName2, sourceFileName3 };

	ConfigurationParser parser(4, argv);
    Configuration configuration = parser.getConfiguration();

	auto sourceFileNames = configuration.getSourceFiles();
	ASSERT_THAT(sourceFileNames, SizeIs(3));
	auto sourceFileNamesIterator = sourceFileNames.begin();
	ASSERT_THAT(*sourceFileNamesIterator, StrEq("test1.c"));
	ASSERT_THAT(*++sourceFileNamesIterator, StrEq("test2.c"));
	ASSERT_THAT(*++sourceFileNamesIterator, StrEq("test3.c"));
}

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
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, grammarArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_THAT(configuration.getGrammarPath(), StrEq("grammar.bnf"));
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.c"));
}

TEST(ConfigurationParser, setsScannerLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-ls";
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, loggingArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_TRUE(configuration.isScannerLoggingEnabled());
	ASSERT_FALSE(configuration.isParserLoggingEnabled());
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.c"));
}

TEST(ConfigurationParser, setsParserLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-lp";
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, loggingArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_TRUE(configuration.isParserLoggingEnabled());
	ASSERT_FALSE(configuration.isScannerLoggingEnabled());
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.c"));
}

TEST(ConfigurationParser, setsParserAndScannerLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-lsp";
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, loggingArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_TRUE(configuration.isParserLoggingEnabled());
	ASSERT_TRUE(configuration.isScannerLoggingEnabled());
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.c"));
}

TEST(ConfigurationParser, terminatesGivenInvalidLoggingArgument) {
	char executable[] = "trans";
	char invalidLoggingArg[] = "-lo";
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, invalidLoggingArg, sourceFileName };

	ASSERT_EXIT(ConfigurationParser configuration(3, argv);, ExitedWithCode(EXIT_FAILURE), "");
}
