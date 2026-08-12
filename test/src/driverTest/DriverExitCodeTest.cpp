#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "driver/Compiler.h"
#include "driver/Configuration.h"
#include "driver/Driver.h"
#include "util/LogManager.h"

#include "ResourceHelpers.h"
#include "DriverHarness.h"
#include "util/Process.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

using namespace testing;

std::filesystem::path writeTempSource(const std::string& name, const std::string& source) {
    auto dir = std::filesystem::temp_directory_path() / "trans_driver_exit_tests";
    std::filesystem::create_directories(dir);
    auto path = dir / name;
    std::ofstream out { path };
    out << source;
    return path;
}

void removeCompileArtifacts(const std::filesystem::path& sourcePath) {
    std::filesystem::remove(sourcePath);
    std::filesystem::remove(sourcePath.string() + ".S");
    std::filesystem::remove(sourcePath.string() + ".o");
    std::filesystem::remove(sourcePath.string() + ".out");
}

// Prepends pathPrefix to PATH for this object's lifetime, then restores it.
class PathGuard {
public:
    explicit PathGuard(const std::string& pathPrefix) {
        const char* previous = std::getenv("PATH");
        if (previous == nullptr) {
            throw std::runtime_error("PATH is not set");
        }
        previousPath_ = previous;
        const std::string combined = pathPrefix + ":" + previousPath_;
        if (setenv("PATH", combined.c_str(), 1) != 0) {
            throw std::runtime_error("setenv(PATH) failed");
        }
    }

    ~PathGuard() {
        setenv("PATH", previousPath_.c_str(), 1);
    }

    PathGuard(const PathGuard&) = delete;
    PathGuard& operator=(const PathGuard&) = delete;

private:
    std::string previousPath_;
};

// Failing `nasm` on PATH (unique temp dir) for the lifetime of this object.
class FakeFailingNasm {
public:
    FakeFailingNasm() :
            binDir_ { makeBinDirWithFailingNasm() },
            pathGuard_ { binDir_.string() } {
    }

    ~FakeFailingNasm() {
        std::error_code ec;
        std::filesystem::remove_all(binDir_, ec);
    }

    FakeFailingNasm(const FakeFailingNasm&) = delete;
    FakeFailingNasm& operator=(const FakeFailingNasm&) = delete;

private:
    static std::filesystem::path makeBinDirWithFailingNasm() {
        std::string templatePath =
                (std::filesystem::temp_directory_path() / "trans_fake_nasm_XXXXXX").string();
        std::vector<char> mutableTemplate(templatePath.begin(), templatePath.end());
        mutableTemplate.push_back('\0');
        char* created = ::mkdtemp(mutableTemplate.data());
        if (created == nullptr) {
            throw std::runtime_error("mkdtemp failed for FakeFailingNasm");
        }
        std::filesystem::path binDir { created };
        const auto nasmPath = binDir / "nasm";
        {
            std::ofstream out { nasmPath };
            out << "#!/bin/sh\n"
                   "echo fake-nasm-failed >&2\n"
                   "exit 1\n";
        }
        std::filesystem::permissions(nasmPath,
                std::filesystem::perms::owner_all
                        | std::filesystem::perms::group_exec
                        | std::filesystem::perms::others_exec);
        return binDir;
    }

    std::filesystem::path binDir_;
    PathGuard pathGuard_;
};

Configuration testConfiguration() {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    return configuration;
}

const char* kTrivialMain =
        "int main() {\n"
        "    return 0;\n"
        "}\n";

TEST(Driver, generateTableWithNoSourcesReturnsZero) {
    std::filesystem::create_directories("logs");
    const auto tablePath = std::filesystem::path { "logs/parsing_table" };
    ArgvBuffer args { {}, { "--grammar=resources/configuration/grammar.bnf" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(tablePath));
    EXPECT_GT(std::filesystem::file_size(tablePath), 0u);
}

TEST(Driver, generateTableWithMissingGrammarReturnsNonZero) {
    ArgvBuffer args { {}, { "--grammar=definitely_missing.bnf" } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("Error:"));
}

TEST(Driver, returnsNonZeroWhenSourceIsMissing) {
    ArgvBuffer args { { "definitely_missing_source_file.c" } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("Error:"));
}

TEST(Driver, returnsZeroForSuccessfulCompile) {
    auto sourcePath = writeTempSource("ok_main.c", kTrivialMain);
    ArgvBuffer args { { sourcePath.string() } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(errors.empty());
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".i"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, returnsNonZeroWhenAnySourceFailsInMultiFileRun) {
    auto goodPath = writeTempSource("multi_ok.c", kTrivialMain);
    auto outPath = goodPath.parent_path() / "multi_fail.out";
    ArgvBuffer args { { goodPath.string(), "definitely_missing_other.c" }, { "-o" + outPath.string() } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("Error:"));
    std::filesystem::remove(outPath);
    removeCompileArtifacts(goodPath);
}

TEST(Driver, compileOnlySkipsLink) {
    auto sourcePath = writeTempSource("compile_only.c", kTrivialMain);
    ArgvBuffer args { { sourcePath.string() }, { "-c" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(sourcePath.string() + ".o"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".i"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".out"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, dashONamesTheExecutable) {
    auto sourcePath = writeTempSource("named_out.c", kTrivialMain);
    auto outPath = sourcePath.parent_path() / "named_hello.out";
    std::filesystem::remove(outPath);
    ArgvBuffer args { { sourcePath.string() }, { "-o" + outPath.string() } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(outPath));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".out"));
    std::filesystem::remove(outPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, compileOnlyWithDashONamesTheObject) {
    auto sourcePath = writeTempSource("named_obj.c", kTrivialMain);
    auto objectPath = sourcePath.parent_path() / "named.o";
    std::filesystem::remove(objectPath);
    ArgvBuffer args { { sourcePath.string() }, { "-c", "-o" + objectPath.string() } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(objectPath));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".o"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".out"));
    std::filesystem::remove(objectPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, compileOnlyWithSeparateDashONamesTheObject) {
    auto sourcePath = writeTempSource("named_obj_sep.c", kTrivialMain);
    auto objectPath = sourcePath.parent_path() / "named_sep.o";
    std::filesystem::remove(objectPath);
    ArgvBuffer args { { sourcePath.string() }, { "-c", "-o", objectPath.string() } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(objectPath));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".o"));
    std::filesystem::remove(objectPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, unknownOptionReturnsNonZeroAndNamesTheFlag) {
    auto sourcePath = writeTempSource("unknown_opt.c", kTrivialMain);
    ArgvBuffer args { { sourcePath.string() }, { "-O2" } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("unknown option"));
    EXPECT_THAT(errors, HasSubstr("-O2"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, helpReturnsZero) {
    ArgvBuffer args { {}, { "-h" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_THAT(errors, HasSubstr("Usage"));
}

TEST(Compiler, preprocessCommandOmitsStdWhenUnset) {
    Configuration configuration;
    auto argv = Compiler::preprocessCommand("in.c", "in.i", configuration);
    EXPECT_THAT(argv, ElementsAre("gcc", "-E", "-x", "c", "-o", "in.i", "in.c"));
}

TEST(Compiler, preprocessCommandPassesPreprocessorStdFlag) {
    Configuration configuration;
    configuration.setPreprocessorStdFlag("c11");
    auto argv = Compiler::preprocessCommand("in.c", "in.i", configuration);
    EXPECT_THAT(argv, ElementsAre("gcc", "-E", "-x", "c", "-std=c11", "-o", "in.i", "in.c"));
}

TEST(Compiler, preprocessCommandOmitsEmptyPreprocessorStdFlag) {
    Configuration configuration;
    configuration.setPreprocessorStdFlag("");
    auto argv = Compiler::preprocessCommand("in.c", "in.i", configuration);
    EXPECT_THAT(argv, ElementsAre("gcc", "-E", "-x", "c", "-o", "in.i", "in.c"));
}

TEST(Compiler, preprocessCommandForwardsPreprocessorArgs) {
    Configuration configuration;
    configuration.setPreprocessorStdFlag("c11");
    configuration.setPreprocessorArgs({ "-I", "inc", "-D", "FOO=2" });
    auto argv = Compiler::preprocessCommand("in.c", "in.i", configuration);
    EXPECT_THAT(argv, ElementsAre(
            "gcc", "-E", "-x", "c", "-std=c11", "-I", "inc", "-D", "FOO=2", "-o", "in.i", "in.c"));
}

TEST(Compiler, preprocessCommandOmitsOutputWhenPathEmpty) {
    Configuration configuration;
    auto argv = Compiler::preprocessCommand("in.c", "", configuration);
    EXPECT_THAT(argv, ElementsAre("gcc", "-E", "-x", "c", "in.c"));
}

TEST(Compiler, preprocessCommandAcceptsMultipleSources) {
    Configuration configuration;
    auto argv = Compiler::preprocessCommand(std::vector<std::string> { "a.c", "b.c" }, "out.i",
            configuration);
    EXPECT_THAT(argv, ElementsAre("gcc", "-E", "-x", "c", "-o", "out.i", "a.c", "b.c"));
}

TEST(Driver, returnsNonZeroWhenCompileOnlyWithObject) {
    auto sourcePath = writeTempSource("mix_src.c", kTrivialMain);
    ArgvBuffer compileArgs { { sourcePath.string() }, { "-c" } };
    std::string errors;
    ASSERT_EQ(runDriver(compileArgs, &errors), 0) << errors;
    auto objectPath = sourcePath.string() + ".o";
    auto otherSource = writeTempSource("mix_other.c", kTrivialMain);
    ArgvBuffer mixArgs { { otherSource.string(), objectPath }, { "-c" } };
    EXPECT_NE(runDriver(mixArgs, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("-c cannot be used with object files"));
    removeCompileArtifacts(sourcePath);
    removeCompileArtifacts(otherSource);
}

TEST(Driver, returnsNonZeroWhenCompileOnlyDashOWithTwoSources) {
    auto first = writeTempSource("c_two_a.c", kTrivialMain);
    auto second = writeTempSource("c_two_b.c", kTrivialMain);
    auto objectPath = first.parent_path() / "c_two.o";
    ArgvBuffer args { { first.string(), second.string() }, { "-c", "-o" + objectPath.string() } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("Error:"));
    EXPECT_FALSE(std::filesystem::exists(objectPath));
    removeCompileArtifacts(first);
    removeCompileArtifacts(second);
}

TEST(Compiler, throwsWhenSourceCannotBeOpened) {
    Compiler compiler { testConfiguration() };
    EXPECT_THROW(compiler.compile("no_such_compile_input.c"), std::runtime_error);
}

// Tool-path unit: assemble failure throws from Compiler (Driver exit code is covered by missing-source cases).
TEST(Compiler, assembleFailureThrowsFromCompile) {
    auto sourcePath = writeTempSource("assemble_fail_compile.c", kTrivialMain);
    FakeFailingNasm fakeNasm;
    Compiler compiler { testConfiguration() };

    try {
        compiler.compile(sourcePath.string());
        FAIL() << "expected compile to throw when nasm fails";
    } catch (const std::runtime_error& error) {
        EXPECT_THAT(std::string(error.what()), HasSubstr("command failed"));
        EXPECT_THAT(std::string(error.what()), AnyOf(HasSubstr("nasm"), HasSubstr("fake-nasm-failed")));
    }

    removeCompileArtifacts(sourcePath);
}

TEST(Driver, dashEPrintsPreprocessedSource) {
    auto sourcePath = writeTempSource("preprocess_stdout.c",
            "#if FOO == 2\npassed\n#endif\nint main(void) { return 0; }\n");
    ArgvBuffer args { { sourcePath.string() }, { "-E", "-DFOO=2" } };
    std::string errors;
    std::string output;
    EXPECT_EQ(runDriver(args, &errors, &output), 0) << errors;
    EXPECT_THAT(output, HasSubstr("passed"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".o"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".S"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, dashEDashOWritesPreprocessedFile) {
    auto sourcePath = writeTempSource("preprocess_out.c",
            "#if FOO == 2\npassed\n#endif\nint main(void) { return 0; }\n");
    auto iPath = sourcePath.parent_path() / "preprocess_out.i";
    std::filesystem::remove(iPath);
    ArgvBuffer args { { sourcePath.string() }, { "-E", "-DFOO=2", "-o", iPath.string() } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    ASSERT_TRUE(std::filesystem::exists(iPath));
    std::ifstream in { iPath };
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_THAT(contents, HasSubstr("passed"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".o"));
    std::filesystem::remove(iPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, dashIFindsLocalHeader) {
    auto incDir = std::filesystem::temp_directory_path() / "trans_driver_exit_tests" / "inc";
    std::filesystem::create_directories(incDir);
    {
        std::ofstream header { incDir / "answer.h" };
        header << "#define ANSWER 7\n";
    }
    auto sourcePath = writeTempSource("include_header.c",
            "#include \"answer.h\"\n"
            "int printf(const char *, ...);\n"
            "int main(void) {\n"
            "    printf(\"%d\", ANSWER);\n"
            "    return 0;\n"
            "}\n");
    auto exePath = sourcePath.parent_path() / "include_header.out";
    std::filesystem::remove(exePath);
    ArgvBuffer args { { sourcePath.string() }, { "-I", incDir.string(), "-o", exePath.string() } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    ASSERT_TRUE(std::filesystem::exists(exePath));
    auto run = util::runProcess({ exePath.string() });
    EXPECT_EQ(run.exitCode, 0) << run.stderrOutput;
    EXPECT_EQ(run.stdoutOutput, "7");
    std::filesystem::remove(exePath);
    std::filesystem::remove(incDir / "answer.h");
    removeCompileArtifacts(sourcePath);
}

} // namespace
