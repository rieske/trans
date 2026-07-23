#include "driver/ConfigurationParser.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <cstdlib>
#include <string>

using namespace testing;

TEST(ConfigurationParser, createsDefaultTransConfiguration) {
	char executable[] = "trans";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, sourceFileName };

	ConfigurationParser parser(2, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_THAT(configuration.getSourceFiles(), SizeIs(1));
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.src"));

	// Grammar path is either the historical relative form (when cwd has
	// resources/) or an absolute path resolved from the binary location
	// (make CC=trans from an out-of-tree directory).
	ASSERT_THAT(configuration.getGrammarPath(),
			EndsWith("resources/configuration/grammar.bnf"));
	ASSERT_THAT(configuration.isScannerLoggingEnabled(), Eq(false));
	ASSERT_THAT(configuration.isParserLoggingEnabled(), Eq(false));
}

TEST(ConfigurationParser, handlesMultipleSourceFiles) {
	char executable[] = "trans";
	char sourceFileName1[] = "test1.src";
	char sourceFileName2[] = "test2.src";
	char sourceFileName3[] = "test3.src";
	char *argv[] = { executable, sourceFileName1, sourceFileName2, sourceFileName3 };

	ConfigurationParser parser(4, argv);
    Configuration configuration = parser.getConfiguration();

	auto sourceFileNames = configuration.getSourceFiles();
	ASSERT_THAT(sourceFileNames, SizeIs(3));
	auto sourceFileNamesIterator = sourceFileNames.begin();
	ASSERT_THAT(*sourceFileNamesIterator, StrEq("test1.src"));
	ASSERT_THAT(*++sourceFileNamesIterator, StrEq("test2.src"));
	ASSERT_THAT(*++sourceFileNamesIterator, StrEq("test3.src"));
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

	ASSERT_EXIT(ConfigurationParser configuration(2, argv), ExitedWithCode(EXIT_FAILURE), "");
}

TEST(ConfigurationParser, setsCustomGrammarFileName) {
	char executable[] = "trans";
	char grammarArg[] = "-ggrammar.bnf";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, grammarArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_THAT(configuration.getGrammarPath(), EndsWith("grammar.bnf"));
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.src"));
}

TEST(ConfigurationParser, acceptsGccStyleFlags) {
	char executable[] = "trans";
	char includeArg[] = "-I/usr/include";
	char defineArg[] = "-DFOO=1";
	char compileOnly[] = "-c";
	char outputArg[] = "-o";
	char outputFile[] = "out.o";
	char sourceFileName[] = "test.c";
	char *argv[] = { executable, includeArg, defineArg, compileOnly, outputArg, outputFile, sourceFileName };

	ConfigurationParser parser(7, argv);
	Configuration configuration = parser.getConfiguration();

	ASSERT_THAT(configuration.getSourceFiles(), SizeIs(1));
	ASSERT_THAT(configuration.getIncludePaths(), ElementsAre(StrEq("/usr/include")));
	ASSERT_THAT(configuration.getDefines(), ElementsAre(StrEq("FOO=1")));
	ASSERT_THAT(configuration.isCompileOnly(), Eq(true));
	ASSERT_THAT(configuration.getOutputFile(), StrEq("out.o"));
}

TEST(ConfigurationParser, ignoresUnknownGccFlags) {
	char executable[] = "trans";
	char wall[] = "-Wall";
	char o2[] = "-O2";
	char sourceFileName[] = "a.c";
	char *argv[] = { executable, wall, o2, sourceFileName };

	ConfigurationParser parser(4, argv);
	Configuration configuration = parser.getConfiguration();

	ASSERT_THAT(configuration.getSourceFiles(), ElementsAre(StrEq("a.c")));
}

// make CC=trans often passes -MD -MF path.d; path must be recorded so the
// driver can write a stub depfile after a successful compile.
TEST(ConfigurationParser, recordsMfDepFileSeparateAndAttached) {
	char executable[] = "trans";
	char md[] = "-MD";
	char mf[] = "-MF";
	char dep[] = "objs/foo.d";
	char sourceFileName[] = "foo.c";
	char *argv[] = { executable, md, mf, dep, sourceFileName };

	ConfigurationParser parser(5, argv);
	Configuration configuration = parser.getConfiguration();

	ASSERT_THAT(configuration.getDepFiles(), ElementsAre(StrEq("objs/foo.d")));

	char mfAttached[] = "-MFbar.d";
	char source2[] = "bar.c";
	char *argv2[] = { executable, mfAttached, source2 };
	ConfigurationParser parser2(3, argv2);
	ASSERT_THAT(parser2.getConfiguration().getDepFiles(), ElementsAre(StrEq("bar.d")));
}

TEST(ConfigurationParser, setsScannerLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-ls";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, loggingArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_TRUE(configuration.isScannerLoggingEnabled());
	ASSERT_FALSE(configuration.isParserLoggingEnabled());
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.src"));
}

TEST(ConfigurationParser, setsParserLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-lp";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, loggingArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_TRUE(configuration.isParserLoggingEnabled());
	ASSERT_FALSE(configuration.isScannerLoggingEnabled());
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.src"));
}

TEST(ConfigurationParser, setsParserAndScannerLogging) {
	char executable[] = "trans";
	char loggingArg[] = "-lsp";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, loggingArg, sourceFileName };

	ConfigurationParser parser(3, argv);
    Configuration configuration = parser.getConfiguration();

	ASSERT_TRUE(configuration.isParserLoggingEnabled());
	ASSERT_TRUE(configuration.isScannerLoggingEnabled());
	ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.src"));
}

TEST(ConfigurationParser, treatsUnknownDashLAsLibraryNotLogging) {
	// -lo is not a logging combo (s|p|t|i); treated as gcc -l<lib> and ignored.
	char executable[] = "trans";
	char libArg[] = "-lo";
	char sourceFileName[] = "test.src";
	char *argv[] = { executable, libArg, sourceFileName };

	ConfigurationParser parser(3, argv);
	Configuration configuration = parser.getConfiguration();
	ASSERT_THAT(configuration.getSourceFiles(), ElementsAre(StrEq("test.src")));
	ASSERT_FALSE(configuration.isScannerLoggingEnabled());
	ASSERT_FALSE(configuration.isParserLoggingEnabled());
}

TEST(ConfigurationParser, setsPreprocessOnly) {
	char executable[] = "trans";
	char eFlag[] = "-E";
	char sourceFileName[] = "a.c";
	char *argv[] = { executable, eFlag, sourceFileName };

	ConfigurationParser parser(3, argv);
	Configuration configuration = parser.getConfiguration();
	ASSERT_TRUE(configuration.isPreprocessOnly());
	ASSERT_FALSE(configuration.isAssemblyOnly());
	ASSERT_FALSE(configuration.isCompileOnly());
}

TEST(ConfigurationParser, setsAssemblyOnly) {
	char executable[] = "trans";
	char sFlag[] = "-S";
	char sourceFileName[] = "a.c";
	char *argv[] = { executable, sFlag, sourceFileName };

	ConfigurationParser parser(3, argv);
	Configuration configuration = parser.getConfiguration();
	ASSERT_TRUE(configuration.isAssemblyOnly());
	ASSERT_FALSE(configuration.isPreprocessOnly());
}

TEST(ConfigurationParser, setsSkipPreprocess) {
	char executable[] = "trans";
	char skip[] = "--no-preprocess";
	char sourceFileName[] = "a.c";
	char *argv[] = { executable, skip, sourceFileName };

	ConfigurationParser parser(3, argv);
	Configuration configuration = parser.getConfiguration();
	ASSERT_TRUE(configuration.shouldSkipPreprocess());
}

TEST(ConfigurationParser, setsUndefine) {
	char executable[] = "trans";
	char uFlag[] = "-UFOO";
	char sourceFileName[] = "a.c";
	char *argv[] = { executable, uFlag, sourceFileName };

	ConfigurationParser parser(3, argv);
	Configuration configuration = parser.getConfiguration();
	ASSERT_THAT(configuration.getUndefines(), ElementsAre(StrEq("FOO")));
}

TEST(ConfigurationParser, setsUndefineSeparateArg) {
	char executable[] = "trans";
	char uFlag[] = "-U";
	char name[] = "BAR";
	char sourceFileName[] = "a.c";
	char *argv[] = { executable, uFlag, name, sourceFileName };

	ConfigurationParser parser(4, argv);
	Configuration configuration = parser.getConfiguration();
	ASSERT_THAT(configuration.getUndefines(), ElementsAre(StrEq("BAR")));
}

TEST(ConfigurationParser, recordsMultipleMfDepFiles) {
	char executable[] = "trans";
	char mf1[] = "-MF";
	char d1[] = "a.d";
	char mf2[] = "-MFmore.d";
	char source[] = "x.c";
	char *argv[] = { executable, mf1, d1, mf2, source };
	ConfigurationParser parser(5, argv);
	EXPECT_THAT(parser.getConfiguration().getDepFiles(),
			ElementsAre(StrEq("a.d"), StrEq("more.d")));
}

TEST(ConfigurationParser, linkOnlyWhenOnlyObjectFiles) {
	char executable[] = "trans";
	char o1[] = "a.o";
	char o2[] = "b.o";
	char outFlag[] = "-o";
	char out[] = "prog";
	char lib[] = "-lm";
	char *argv[] = { executable, outFlag, out, o1, o2, lib };
	ConfigurationParser parser(6, argv);
	Configuration c = parser.getConfiguration();
	EXPECT_TRUE(c.isLinkOnly());
	EXPECT_THAT(c.getObjectFiles(), ElementsAre(StrEq("a.o"), StrEq("b.o")));
	EXPECT_THAT(c.getOutputFile(), StrEq("prog"));
	EXPECT_THAT(c.getLinkArgs(), ElementsAre(StrEq("-lm")));
}

TEST(ConfigurationParser, ignoresMdButRecordsMf) {
	char executable[] = "trans";
	char md[] = "-MD";
	char mmd[] = "-MMD";
	char mf[] = "-MF";
	char dep[] = "x.d";
	char source[] = "x.c";
	char *argv[] = { executable, md, mmd, mf, dep, source };
	ConfigurationParser parser(6, argv);
	EXPECT_THAT(parser.getConfiguration().getDepFiles(), ElementsAre(StrEq("x.d")));
	EXPECT_FALSE(parser.getConfiguration().isLinkOnly());
}

TEST(ConfigurationParser, keepsWlAndLForLink) {
	char executable[] = "trans";
	char wl[] = "-Wl,-rpath,/usr/lib";
	char L[] = "-L/opt/lib";
	char o[] = "a.o";
	char *argv[] = { executable, wl, L, o };
	ConfigurationParser parser(4, argv);
	Configuration c = parser.getConfiguration();
	ASSERT_TRUE(c.isLinkOnly());
	EXPECT_THAT(c.getLinkArgs(), ElementsAre(StrEq("-Wl,-rpath,/usr/lib"), StrEq("-L/opt/lib")));
}

// TRANS_RESOURCES is honored when -r is not given (make CC=trans out-of-tree).
TEST(ConfigurationParser, usesTransResourcesEnvWhenNoRFlag) {
	char executable[] = "trans";
	char source[] = "x.c";
	char *argv[] = { executable, source };

	const char* prev = std::getenv("TRANS_RESOURCES");
	std::string prevCopy = prev ? prev : "";
	setenv("TRANS_RESOURCES", "/tmp/trans_resources_unit_test/", 1);

	ConfigurationParser parser(2, argv);
	Configuration c = parser.getConfiguration();
	EXPECT_THAT(c.getResourcesBasePath(), StrEq("/tmp/trans_resources_unit_test/"));

	if (prev) {
		setenv("TRANS_RESOURCES", prevCopy.c_str(), 1);
	} else {
		unsetenv("TRANS_RESOURCES");
	}
}

// Explicit -r wins over TRANS_RESOURCES.
TEST(ConfigurationParser, explicitRFlagOverridesTransResourcesEnv) {
	char executable[] = "trans";
	char rFlag[] = "-r";
	char rPath[] = "/explicit/resources/";
	char source[] = "x.c";
	char *argv[] = { executable, rFlag, rPath, source };

	const char* prev = std::getenv("TRANS_RESOURCES");
	std::string prevCopy = prev ? prev : "";
	setenv("TRANS_RESOURCES", "/tmp/trans_resources_should_not_win/", 1);

	ConfigurationParser parser(4, argv);
	Configuration c = parser.getConfiguration();
	EXPECT_THAT(c.getResourcesBasePath(), StrEq("/explicit/resources/"));

	if (prev) {
		setenv("TRANS_RESOURCES", prevCopy.c_str(), 1);
	} else {
		unsetenv("TRANS_RESOURCES");
	}
}
