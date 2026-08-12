#include "driver/ConfigurationParser.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <cstdlib>
#include <optional>
#include <string>
#include <vector>

using namespace testing;

namespace {

class Argv {
public:
    explicit Argv(std::initializer_list<const char*> args) {
        for (const char* arg : args) {
            storage_.push_back(arg);
        }
        for (auto& s : storage_) {
            ptrs_.push_back(s.data());
        }
    }

    int argc() const {
        return static_cast<int>(ptrs_.size());
    }

    char** argv() {
        return ptrs_.data();
    }

private:
    std::vector<std::string> storage_;
    std::vector<char*> ptrs_;
};

class EnvGuard {
public:
    EnvGuard(const char* key, const char* value) : key_ { key } {
        if (const char* previous = std::getenv(key)) {
            previous_ = previous;
        }
        if (value == nullptr) {
            unsetenv(key);
        } else {
            setenv(key, value, 1);
        }
    }

    ~EnvGuard() {
        if (previous_) {
            setenv(key_.c_str(), previous_->c_str(), 1);
        } else {
            unsetenv(key_.c_str());
        }
    }

    EnvGuard(const EnvGuard&) = delete;
    EnvGuard& operator=(const EnvGuard&) = delete;

private:
    std::string key_;
    std::optional<std::string> previous_;
};

ParseResult parse(std::initializer_list<const char*> args) {
    Argv argv { args };
    return parseCommandLine(argv.argc(), argv.argv());
}

bool succeeded(const ParseResult& result) {
    return result.configuration.has_value() && result.exitCode == 0;
}

bool failed(const ParseResult& result) {
    return !result.configuration.has_value() && result.exitCode != 0;
}

bool isHelp(const ParseResult& result) {
    return !result.configuration.has_value() && result.exitCode == 0;
}

const Configuration& config(const ParseResult& result) {
    return *result.configuration;
}

class ConfigurationParserTest : public Test {
protected:
    EnvGuard resourcesEnv { "TRANS_RESOURCES", nullptr };
    EnvGuard logEnv { "TRANS_LOG", nullptr };
};

} // namespace

TEST_F(ConfigurationParserTest, createsDefaultTransConfiguration) {
    auto result = parse({ "trans", "test.c" });

    ASSERT_TRUE(succeeded(result));
    const Configuration& configuration = config(result);
    ASSERT_THAT(configuration.getSourceFiles(), SizeIs(1));
    ASSERT_THAT(*configuration.getSourceFiles().begin(), StrEq("test.c"));
    ASSERT_THAT(configuration.getGrammarPath(), StrEq("resources/configuration/grammar.bnf"));
    ASSERT_THAT(configuration.isScannerLoggingEnabled(), Eq(false));
    ASSERT_THAT(configuration.isParserLoggingEnabled(), Eq(false));
    ASSERT_THAT(configuration.getAssemblyDialect(), Eq(AssemblyDialect::Intel));
    ASSERT_THAT(configuration.assemblyDialectTag(), StrEq("intel"));
    ASSERT_THAT(configuration.gnuExtensions(), Eq(true));
    ASSERT_THAT(configuration.getPreprocessorStdFlag(), StrEq(""));
}

TEST_F(ConfigurationParserTest, isoAliasDoesNotForwardStdToPreprocessor) {
    auto result = parse({ "trans", "-std=c", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).gnuExtensions(), Eq(false));
    ASSERT_THAT(config(result).getPreprocessorStdFlag(), StrEq(""));
}

TEST_F(ConfigurationParserTest, gnuAliasDoesNotForwardStdToPreprocessor) {
    auto result = parse({ "trans", "-std=gnu", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).gnuExtensions(), Eq(true));
    ASSERT_THAT(config(result).getPreprocessorStdFlag(), StrEq(""));
}

TEST_F(ConfigurationParserTest, yearQualifiedIsoStdIsForwarded) {
    auto result = parse({ "trans", "-std=c11", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).gnuExtensions(), Eq(false));
    ASSERT_THAT(config(result).getPreprocessorStdFlag(), StrEq("c11"));
}

TEST_F(ConfigurationParserTest, yearQualifiedGnuStdIsForwarded) {
    auto result = parse({ "trans", "-std=gnu17", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).gnuExtensions(), Eq(true));
    ASSERT_THAT(config(result).getPreprocessorStdFlag(), StrEq("gnu17"));
}

TEST_F(ConfigurationParserTest, acceptsC89AsIsoAndForwardsIt) {
    auto result = parse({ "trans", "-std=c89", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).gnuExtensions(), Eq(false));
    ASSERT_THAT(config(result).getPreprocessorStdFlag(), StrEq("c89"));
}

TEST_F(ConfigurationParserTest, lastStdWins) {
    auto result = parse({ "trans", "-std=c11", "-std=gnu17", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).gnuExtensions(), Eq(true));
    ASSERT_THAT(config(result).getPreprocessorStdFlag(), StrEq("gnu17"));
}

TEST_F(ConfigurationParserTest, shortDashMIsUnknownNotAMasmPrefix) {
    auto result = parse({ "trans", "-m", "test.c" });
    ASSERT_TRUE(failed(result));
    ASSERT_THAT(result.message, HasSubstr("unknown option"));
    ASSERT_THAT(result.message, HasSubstr("-m"));
}

TEST_F(ConfigurationParserTest, masmIsNotStolenByAShorterUnknownPrefix) {
    auto result = parse({ "trans", "-masm=att", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getAssemblyDialect(), Eq(AssemblyDialect::AtAndT));
}

TEST_F(ConfigurationParserTest, rejectsUnknownLanguageStd) {
    auto result = parse({ "trans", "-std=c++11", "test.c" });
    ASSERT_TRUE(failed(result));
    ASSERT_EQ(result.exitCode, 1);
    ASSERT_THAT(result.message, HasSubstr("c++11"));
}

TEST_F(ConfigurationParserTest, stdWithoutValueIsMissingArgument) {
    auto result = parse({ "trans", "-std", "test.c" });
    ASSERT_TRUE(failed(result));
    ASSERT_THAT(result.message, HasSubstr("missing argument"));
    ASSERT_THAT(result.message, HasSubstr("-std"));
}

TEST_F(ConfigurationParserTest, setsIntelAssemblyDialect) {
    auto result = parse({ "trans", "-masm=intel", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getAssemblyDialect(), Eq(AssemblyDialect::Intel));
    ASSERT_THAT(config(result).assemblyDialectTag(), StrEq("intel"));
}

TEST_F(ConfigurationParserTest, setsAtAndTAssemblyDialect) {
    auto result = parse({ "trans", "-masm=att", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getAssemblyDialect(), Eq(AssemblyDialect::AtAndT));
    ASSERT_THAT(config(result).assemblyDialectTag(), StrEq("att"));
}

TEST_F(ConfigurationParserTest, acceptsSeparateMasmArgument) {
    auto result = parse({ "trans", "-masm", "att", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getAssemblyDialect(), Eq(AssemblyDialect::AtAndT));
}

TEST_F(ConfigurationParserTest, rejectsUnknownAssemblyDialect) {
    auto result = parse({ "trans", "-masm=gas", "test.c" });
    ASSERT_TRUE(failed(result));
    ASSERT_EQ(result.exitCode, 1);
    ASSERT_THAT(result.message, HasSubstr("gas"));
}

TEST_F(ConfigurationParserTest, oldDashAFlagIsUnknown) {
    auto result = parse({ "trans", "-aintel", "test.c" });
    ASSERT_TRUE(failed(result));
    ASSERT_THAT(result.message, HasSubstr("unknown option"));
    ASSERT_THAT(result.message, HasSubstr("-aintel"));
}

TEST_F(ConfigurationParserTest, handlesMultipleSourceFiles) {
    auto result = parse({ "trans", "test1.c", "test2.c", "test3.c" });
    ASSERT_TRUE(succeeded(result));
    auto sourceFileNames = config(result).getSourceFiles();
    ASSERT_THAT(sourceFileNames, SizeIs(3));
    ASSERT_THAT(sourceFileNames[0], StrEq("test1.c"));
    ASSERT_THAT(sourceFileNames[1], StrEq("test2.c"));
    ASSERT_THAT(sourceFileNames[2], StrEq("test3.c"));
}

TEST_F(ConfigurationParserTest, acceptsOptionAfterSourceFile) {
    auto result = parse({ "trans", "test.c", "-c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isCompileOnly());
    ASSERT_THAT(*config(result).getSourceFiles().begin(), StrEq("test.c"));
}

TEST_F(ConfigurationParserTest, doubleDashStopsOptionParsing) {
    auto result = parse({ "trans", "--", "-c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_FALSE(config(result).isCompileOnly());
    ASSERT_THAT(*config(result).getSourceFiles().begin(), StrEq("-c"));
}

TEST_F(ConfigurationParserTest, failsForNullArgv) {
    auto result = parseCommandLine(0, nullptr);
    ASSERT_TRUE(failed(result));
    ASSERT_EQ(result.exitCode, 1);
}

TEST_F(ConfigurationParserTest, failsIfNoSourceFilesSpecified) {
    auto result = parse({ "trans" });
    ASSERT_TRUE(failed(result));
    ASSERT_EQ(result.exitCode, 1);
    ASSERT_THAT(result.message, HasSubstr("no input files"));
}

TEST_F(ConfigurationParserTest, helpHasNoConfiguration) {
    auto result = parse({ "trans", "-h" });
    ASSERT_TRUE(isHelp(result));
    ASSERT_THAT(result.message, HasSubstr("Usage"));
    ASSERT_THAT(result.message, HasSubstr("-masm=intel|att"));
    ASSERT_THAT(result.message, HasSubstr("--grammar"));
    ASSERT_THAT(result.message, HasSubstr("--resources"));
    ASSERT_THAT(result.message, HasSubstr("-v"));
    ASSERT_THAT(result.message, HasSubstr("ignored"));
}

TEST_F(ConfigurationParserTest, longHelpIsTheSameAsDashH) {
    auto result = parse({ "trans", "--help" });
    ASSERT_TRUE(isHelp(result));
}

TEST_F(ConfigurationParserTest, helpIgnoresInvalidLogEnvironment) {
    EnvGuard log { "TRANS_LOG", "o" };
    auto result = parse({ "trans", "-h" });
    ASSERT_TRUE(isHelp(result));
}

TEST_F(ConfigurationParserTest, unknownOptionIsAnError) {
    auto result = parse({ "trans", "--not-a-flag", "test.c" });
    ASSERT_TRUE(failed(result));
    ASSERT_THAT(result.message, HasSubstr("unknown option"));
    ASSERT_THAT(result.message, HasSubstr("--not-a-flag"));
}

TEST_F(ConfigurationParserTest, makefileCflagsAreIgnored) {
    auto result = parse({
            "trans", "-O2", "-g", "-Wall", "-Wextra", "-Werror", "-pipe", "-fPIC", "-c", "test.c"
    });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isCompileOnly());
    ASSERT_THAT(config(result).getPreprocessorArgs(), IsEmpty());
    ASSERT_THAT(config(result).getIgnoredFlags(),
            ElementsAre("-O2", "-g", "-Wall", "-Wextra", "-Werror", "-pipe", "-fPIC"));
    ASSERT_FALSE(config(result).isVerbose());
}

TEST_F(ConfigurationParserTest, ignoredFlagsDoNotEatFollowingTokens) {
    auto result = parse({ "trans", "-O", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(*config(result).getSourceFiles().begin(), StrEq("test.c"));
}

TEST_F(ConfigurationParserTest, warningIgnoreDoesNotSwallowWl) {
    auto result = parse({ "trans", "-Wl,-as-needed", "test.c" });
    ASSERT_TRUE(failed(result));
    ASSERT_THAT(result.message, HasSubstr("unknown option"));
    ASSERT_THAT(result.message, HasSubstr("-Wl,-as-needed"));
}

TEST_F(ConfigurationParserTest, dashIStillForwardedAmongIgnoredFlags) {
    auto result = parse({ "trans", "-O2", "-Iinc", "-Wall", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getPreprocessorArgs(), ElementsAre("-I", "inc"));
    ASSERT_THAT(config(result).getIgnoredFlags(), ElementsAre("-O2", "-Wall"));
}

TEST_F(ConfigurationParserTest, verboseIsLastWinsAssign) {
    auto result = parse({ "trans", "-v", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isVerbose());
}

TEST_F(ConfigurationParserTest, oldDashLFlagIsUnknown) {
    auto result = parse({ "trans", "-l", "test.c" });
    ASSERT_TRUE(failed(result));
    ASSERT_THAT(result.message, HasSubstr("unknown option"));
    ASSERT_THAT(result.message, HasSubstr("-l"));
}

TEST_F(ConfigurationParserTest, setsCompileOnly) {
    auto result = parse({ "trans", "-c", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isCompileOnly());
    ASSERT_THAT(*config(result).getSourceFiles().begin(), StrEq("test.c"));
}

TEST_F(ConfigurationParserTest, setsOutputPathStuckToFlag) {
    auto result = parse({ "trans", "-oout.exe", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getOutputPath(), StrEq("out.exe"));
    ASSERT_THAT(*config(result).getSourceFiles().begin(), StrEq("test.c"));
}

TEST_F(ConfigurationParserTest, setsOutputPathAsSeparateArgument) {
    auto result = parse({ "trans", "-o", "out.exe", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getOutputPath(), StrEq("out.exe"));
}

TEST_F(ConfigurationParserTest, lastOutputPathWins) {
    auto result = parse({ "trans", "-o", "first.exe", "-o", "second.exe", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getOutputPath(), StrEq("second.exe"));
}

TEST_F(ConfigurationParserTest, lastResourcesAssignmentWins) {
    EnvGuard resources { "TRANS_RESOURCES", "from-env/" };
    auto result = parse({ "trans", "--resources=first/", "--resources=second/", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getLexPath(),
            StrEq("second/resources/configuration/scanner.lex"));
}

TEST_F(ConfigurationParserTest, missingOutputPathArgumentIsAnError) {
    auto result = parse({ "trans", "-o" });
    ASSERT_TRUE(failed(result));
    ASSERT_THAT(result.message, HasSubstr("missing argument"));
    ASSERT_THAT(result.message, HasSubstr("-o"));
}

TEST_F(ConfigurationParserTest, acceptsOutputPathWithMultipleFiles) {
    auto result = parse({ "trans", "-o", "out.exe", "test1.c", "test2.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getOutputPath(), StrEq("out.exe"));
    ASSERT_THAT(config(result).getSourceFiles(), SizeIs(2));
}

TEST_F(ConfigurationParserTest, setsCompileOnlyAndOutputPath) {
    auto result = parse({ "trans", "-c", "-o", "obj.o", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isCompileOnly());
    ASSERT_THAT(config(result).getOutputPath(), StrEq("obj.o"));
    ASSERT_THAT(*config(result).getSourceFiles().begin(), StrEq("test.c"));
}

TEST_F(ConfigurationParserTest, setsCustomGrammarFileName) {
    auto result = parse({ "trans", "--grammar=grammar.bnf", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getGrammarPath(), StrEq("grammar.bnf"));
    ASSERT_TRUE(config(result).usingCustomGrammar());
    ASSERT_THAT(*config(result).getSourceFiles().begin(), StrEq("test.c"));
}

TEST_F(ConfigurationParserTest, grammarWithoutSourcesIsAllowed) {
    auto result = parse({ "trans", "--grammar", "grammar.bnf" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).usingCustomGrammar());
    ASSERT_THAT(config(result).getSourceFiles(), IsEmpty());
}

TEST_F(ConfigurationParserTest, setsResourcesPath) {
    auto result = parse({ "trans", "--resources=myres/", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getLexPath(), StrEq("myres/resources/configuration/scanner.lex"));
}

TEST_F(ConfigurationParserTest, resourcesEnvironmentIsUsedWhenFlagMissing) {
    EnvGuard resources { "TRANS_RESOURCES", "from-env/" };
    auto result = parse({ "trans", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getLexPath(),
            StrEq("from-env/resources/configuration/scanner.lex"));
}

TEST_F(ConfigurationParserTest, resourcesFlagOverridesEnvironment) {
    EnvGuard resources { "TRANS_RESOURCES", "from-env/" };
    auto result = parse({ "trans", "--resources=from-cli/", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getLexPath(),
            StrEq("from-cli/resources/configuration/scanner.lex"));
}

TEST_F(ConfigurationParserTest, setsScannerLogging) {
    auto result = parse({ "trans", "--log=scanner", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isScannerLoggingEnabled());
    ASSERT_FALSE(config(result).isParserLoggingEnabled());
}

TEST_F(ConfigurationParserTest, setsParserLogging) {
    auto result = parse({ "trans", "--log=parser", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isParserLoggingEnabled());
    ASSERT_FALSE(config(result).isScannerLoggingEnabled());
}

TEST_F(ConfigurationParserTest, setsParserAndScannerLogging) {
    auto result = parse({ "trans", "--log=scanner,parser", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isParserLoggingEnabled());
    ASSERT_TRUE(config(result).isScannerLoggingEnabled());
}

TEST_F(ConfigurationParserTest, acceptsShortLogComponentNames) {
    auto result = parse({ "trans", "--log=s,p", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isScannerLoggingEnabled());
    ASSERT_TRUE(config(result).isParserLoggingEnabled());
}

TEST_F(ConfigurationParserTest, rejectsInvalidLogComponent) {
    auto result = parse({ "trans", "--log=o", "test.c" });
    ASSERT_TRUE(failed(result));
    ASSERT_THAT(result.message, HasSubstr("o"));
}

TEST_F(ConfigurationParserTest, logEnvironmentIsUsedWhenFlagMissing) {
    EnvGuard log { "TRANS_LOG", "scanner" };
    auto result = parse({ "trans", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isScannerLoggingEnabled());
    ASSERT_FALSE(config(result).isParserLoggingEnabled());
}

TEST_F(ConfigurationParserTest, logFlagOverridesInvalidEnvironment) {
    EnvGuard log { "TRANS_LOG", "o" };
    auto result = parse({ "trans", "--log=scanner", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isScannerLoggingEnabled());
}

TEST_F(ConfigurationParserTest, lastLogAssignmentWins) {
    auto result = parse({ "trans", "--log=parser", "--log=scanner", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isScannerLoggingEnabled());
    ASSERT_FALSE(config(result).isParserLoggingEnabled());
}

TEST_F(ConfigurationParserTest, includePathSeparateAndStuckAreForwarded) {
    auto result = parse({ "trans", "-I", "inc", "-Ilib", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getPreprocessorArgs(), ElementsAre("-I", "inc", "-I", "lib"));
    ASSERT_FALSE(config(result).isCompileOnly());
    ASSERT_FALSE(config(result).isPreprocessOnly());
}

TEST_F(ConfigurationParserTest, defineAndUndefineAreForwardedInOrder) {
    auto result = parse({ "trans", "-DFOO=2", "-U", "BAR", "-D", "BAZ", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getPreprocessorArgs(),
            ElementsAre("-D", "FOO=2", "-U", "BAR", "-D", "BAZ"));
}

TEST_F(ConfigurationParserTest, includeIsystemIquoteAreForwarded) {
    auto result = parse({
            "trans", "-include", "hdr.h", "-isystem", "/usr/include", "-iquote", "q", "test.c"
    });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getPreprocessorArgs(),
            ElementsAre("-include", "hdr.h", "-isystem", "/usr/include", "-iquote", "q"));
}

TEST_F(ConfigurationParserTest, includeIsNotAnIStuckPrefix) {
    auto result = parse({ "trans", "-include", "hdr.h", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_THAT(config(result).getPreprocessorArgs(), ElementsAre("-include", "hdr.h"));
}

TEST_F(ConfigurationParserTest, missingIncludePathIsAnError) {
    auto result = parse({ "trans", "-I" });
    ASSERT_TRUE(failed(result));
    ASSERT_THAT(result.message, HasSubstr("missing argument"));
    ASSERT_THAT(result.message, HasSubstr("-I"));
}

TEST_F(ConfigurationParserTest, dashESetsPreprocessOnly) {
    auto result = parse({ "trans", "-E", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isPreprocessOnly());
    ASSERT_FALSE(config(result).isCompileOnly());
}

TEST_F(ConfigurationParserTest, dashEWinsOverDashC) {
    auto result = parse({ "trans", "-c", "-E", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isPreprocessOnly());
}

TEST_F(ConfigurationParserTest, dashEThenDashCStillPreprocessOnly) {
    auto result = parse({ "trans", "-E", "-c", "test.c" });
    ASSERT_TRUE(succeeded(result));
    ASSERT_TRUE(config(result).isPreprocessOnly());
}
