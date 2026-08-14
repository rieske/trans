#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "driver/Compiler.h"
#include "driver/Configuration.h"
#include "driver/Driver.h"
#include "util/LogManager.h"

#include "ResourceHelpers.h"
#include "DriverHarness.h"
#include "util/Process.h"
#include "util/SourcePath.h"

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
    std::filesystem::remove(sourcePath.string() + ".s");
    std::filesystem::remove(sourcePath.string() + ".i");
    std::filesystem::remove(sourcePath.string() + ".o");
    std::filesystem::remove(sourcePath.string() + ".out");
    auto stem = sourcePath;
    stem.replace_extension();
    std::filesystem::remove(stem.string() + ".s");
    std::filesystem::remove(stem.string() + ".i");
    std::filesystem::remove(stem.string() + ".o");
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
    ArgvBuffer args { {}, { "--grammar=" + getResourcePath("configuration/grammar.bnf") } };
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

TEST(Driver, findsResourcesWhenCwdHasNone) {
    const auto trans = std::filesystem::absolute(transBinaryPath());
    auto dir = std::filesystem::temp_directory_path() / "trans_foreign_cwd";
    std::filesystem::create_directories(dir);
    auto sourcePath = dir / "ok.c";
    auto objectPath = dir / "ok.o";
    {
        std::ofstream out { sourcePath };
        out << kTrivialMain;
    }
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(dir);
    const auto result = util::runProcess({
            trans.string(), "-c", "-o", objectPath.string(), sourcePath.string() });
    std::filesystem::current_path(previous);
    EXPECT_EQ(result.exitCode, 0) << result.stderrOutput;
    EXPECT_THAT(result.stderrOutput, Not(HasSubstr("grammar.bnf")));
    EXPECT_TRUE(std::filesystem::exists(objectPath));
    std::filesystem::remove(objectPath);
    std::filesystem::remove(sourcePath);
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
    ArgvBuffer args { { sourcePath.string() }, { "--not-a-flag" } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("unknown option"));
    EXPECT_THAT(errors, HasSubstr("--not-a-flag"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, makefileCflagsCompileOnlySucceedsSilently) {
    auto sourcePath = writeTempSource("cflags_silent.c", kTrivialMain);
    ArgvBuffer args { { sourcePath.string() }, { "-O2", "-g", "-Wall", "-c" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_THAT(errors, Not(HasSubstr("ignoring")));
    EXPECT_TRUE(std::filesystem::exists(sourcePath.string() + ".o"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, verbosePrintsIgnoredFlags) {
    auto sourcePath = writeTempSource("cflags_verbose.c", kTrivialMain);
    ArgvBuffer args { { sourcePath.string() }, { "-v", "-O2", "-c" } };
    std::string errors;
    std::string output;
    EXPECT_EQ(runDriver(args, &errors, &output), 0) << errors;
    EXPECT_THAT(errors, HasSubstr("ignoring -O2"));
    EXPECT_THAT(output, HasSubstr("Compiling"));
    EXPECT_TRUE(std::filesystem::exists(sourcePath.string() + ".o"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, compileOnlyDoesNotLeaveAssemblyBesideSource) {
    auto sourcePath = writeTempSource("no_leftover_asm.c", kTrivialMain);
    ArgvBuffer args { { sourcePath.string() }, { "-c" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(sourcePath.string() + ".o"));
    auto stem = sourcePath;
    stem.replace_extension();
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".S"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".s"));
    EXPECT_FALSE(std::filesystem::exists(stem.string() + ".s"));
    EXPECT_FALSE(std::filesystem::exists(stem.string() + ".i"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, dashSWritesAssemblyAndSkipsObject) {
    auto sourcePath = writeTempSource("asm_only.c", kTrivialMain);
    auto stem = sourcePath;
    stem.replace_extension();
    auto asmPath = stem.string() + ".s";
    std::filesystem::remove(asmPath);
    ArgvBuffer args { { sourcePath.string() }, { "-S" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(asmPath));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".o"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".out"));
    std::filesystem::remove(asmPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, dashSWithDashONamesTheAssembly) {
    auto sourcePath = writeTempSource("asm_named.c", kTrivialMain);
    auto outPath = sourcePath.parent_path() / "named_out.s";
    std::filesystem::remove(outPath);
    ArgvBuffer args { { sourcePath.string() }, { "-S", "-o", outPath.string() } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(outPath));
    auto stem = sourcePath;
    stem.replace_extension();
    EXPECT_FALSE(std::filesystem::exists(stem.string() + ".s"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".o"));
    std::filesystem::remove(outPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, dashSWithObjectInputFails) {
    auto sourcePath = writeTempSource("asm_mix.c", kTrivialMain);
    ArgvBuffer compileArgs { { sourcePath.string() }, { "-c" } };
    std::string errors;
    ASSERT_EQ(runDriver(compileArgs, &errors), 0) << errors;
    auto objectPath = sourcePath.string() + ".o";
    ArgvBuffer mixArgs { { objectPath }, { "-S" } };
    EXPECT_NE(runDriver(mixArgs, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("-S cannot be used with object files"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, dashSDashOWithTwoSourcesFails) {
    auto first = writeTempSource("asm_two_a.c", kTrivialMain);
    auto second = writeTempSource("asm_two_b.c", kTrivialMain);
    auto outPath = first.parent_path() / "asm_two.s";
    ArgvBuffer args { { first.string(), second.string() }, { "-S", "-o", outPath.string() } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("Error:"));
    EXPECT_FALSE(std::filesystem::exists(outPath));
    removeCompileArtifacts(first);
    removeCompileArtifacts(second);
}

TEST(Driver, saveTempsKeepsIntermediatesBesideSource) {
    // Source must need the preprocessor so -save-temps has a real .i to keep.
    auto sourcePath = writeTempSource("save_temps.c",
            "#define Z 0\nint main(void) { return Z; }\n");
    auto stem = sourcePath;
    stem.replace_extension();
    auto iPath = stem.string() + ".i";
    auto sPath = stem.string() + ".s";
    std::filesystem::remove(iPath);
    std::filesystem::remove(sPath);
    ArgvBuffer args { { sourcePath.string() }, { "-c", "-save-temps" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(sourcePath.string() + ".o"));
    EXPECT_TRUE(std::filesystem::exists(iPath));
    EXPECT_TRUE(std::filesystem::exists(sPath));
    std::filesystem::remove(iPath);
    std::filesystem::remove(sPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, saveTempsWithoutPreprocessorKeepsAssemblyOnly) {
    auto sourcePath = writeTempSource("save_temps_no_cpp.c", kTrivialMain);
    auto stem = sourcePath;
    stem.replace_extension();
    auto iPath = stem.string() + ".i";
    auto sPath = stem.string() + ".s";
    std::filesystem::remove(iPath);
    std::filesystem::remove(sPath);
    ArgvBuffer args { { sourcePath.string() }, { "-c", "-save-temps" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(sourcePath.string() + ".o"));
    EXPECT_FALSE(std::filesystem::exists(iPath));
    EXPECT_TRUE(std::filesystem::exists(sPath));
    std::filesystem::remove(sPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, preprocessedInputSkipsGccE) {
    auto sourcePath = writeTempSource("from_i_input.c", kTrivialMain);
    auto stem = sourcePath;
    stem.replace_extension();
    auto iPath = stem.string() + ".i";
    ArgvBuffer preprocessArgs { { sourcePath.string() }, { "-E", "-o", iPath } };
    std::string errors;
    ASSERT_EQ(runDriver(preprocessArgs, &errors), 0) << errors;
    ASSERT_TRUE(std::filesystem::exists(iPath));
    std::filesystem::remove(sourcePath);
    auto objectPath = stem.string() + ".o";
    std::filesystem::remove(objectPath);
    ArgvBuffer compileArgs { { iPath }, { "-c", "-o", objectPath } };
    EXPECT_EQ(runDriver(compileArgs, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(objectPath));
    std::filesystem::remove(iPath);
    std::filesystem::remove(objectPath);
}

TEST(Driver, assemblyInputAssemblesToObject) {
    auto sourcePath = writeTempSource("asm_input.c", kTrivialMain);
    auto stem = sourcePath;
    stem.replace_extension();
    auto sPath = stem.string() + ".s";
    ArgvBuffer asmArgs { { sourcePath.string() }, { "-S", "-o", sPath } };
    std::string errors;
    ASSERT_EQ(runDriver(asmArgs, &errors), 0) << errors;
    ASSERT_TRUE(std::filesystem::exists(sPath));
    auto objectPath = stem.string() + "_from_s.o";
    std::filesystem::remove(objectPath);
    ArgvBuffer objArgs { { sPath }, { "-c", "-o", objectPath } };
    EXPECT_EQ(runDriver(objArgs, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(objectPath));
    std::filesystem::remove(sPath);
    std::filesystem::remove(objectPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, dashSWithAssemblyInputFails) {
    auto sourcePath = writeTempSource("asm_s_input.c", kTrivialMain);
    auto stem = sourcePath;
    stem.replace_extension();
    auto sPath = stem.string() + ".s";
    ArgvBuffer asmArgs { { sourcePath.string() }, { "-S", "-o", sPath } };
    std::string errors;
    ASSERT_EQ(runDriver(asmArgs, &errors), 0) << errors;
    ArgvBuffer sArgs { { sPath }, { "-S" } };
    EXPECT_NE(runDriver(sArgs, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("-S cannot be used with assembly files"));
    std::filesystem::remove(sPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, singleAssemblyInputLinksWithDefaultExecutableName) {
    auto sourcePath = writeTempSource("asm_default_link.c", kTrivialMain);
    auto stem = sourcePath;
    stem.replace_extension();
    auto sPath = stem.string() + ".s";
    ArgvBuffer asmArgs { { sourcePath.string() }, { "-S", "-o", sPath } };
    std::string errors;
    ASSERT_EQ(runDriver(asmArgs, &errors), 0) << errors;
    auto exePath = sPath + ".out";
    std::filesystem::remove(exePath);
    ArgvBuffer linkArgs { { sPath } };
    EXPECT_EQ(runDriver(linkArgs, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(exePath));
    std::filesystem::remove(exePath);
    std::filesystem::remove(sPath + ".o");
    std::filesystem::remove(sPath);
    removeCompileArtifacts(sourcePath);
}

namespace {

// Isolates TMPDIR so intermediate temps land in a private directory we can inspect.
class IsolatedTmpDir {
public:
    IsolatedTmpDir() {
        const char* previous = std::getenv("TMPDIR");
        if (previous != nullptr) {
            previousTmpdir_ = previous;
        }
        std::string templatePath =
                (std::filesystem::temp_directory_path() / "trans_isolated_XXXXXX").string();
        std::vector<char> mutableTemplate(templatePath.begin(), templatePath.end());
        mutableTemplate.push_back('\0');
        char* created = ::mkdtemp(mutableTemplate.data());
        if (created == nullptr) {
            throw std::runtime_error("mkdtemp failed for IsolatedTmpDir");
        }
        dir_ = created;
        if (setenv("TMPDIR", dir_.c_str(), 1) != 0) {
            throw std::runtime_error("setenv(TMPDIR) failed");
        }
    }

    ~IsolatedTmpDir() {
        if (previousTmpdir_.empty()) {
            unsetenv("TMPDIR");
        } else {
            setenv("TMPDIR", previousTmpdir_.c_str(), 1);
        }
        std::error_code ec;
        std::filesystem::remove_all(dir_, ec);
    }

    IsolatedTmpDir(const IsolatedTmpDir&) = delete;
    IsolatedTmpDir& operator=(const IsolatedTmpDir&) = delete;

    const std::filesystem::path& path() const { return dir_; }

private:
    std::string previousTmpdir_;
    std::filesystem::path dir_;
};

bool isCompilerTempName(const std::string& filename) {
    // mkstemps templates are "transXXXXXX" + suffix (.i / .s).
    if (filename.size() < 6 || filename.compare(0, 5, "trans") != 0) {
        return false;
    }
    return util::hasSuffix(filename, ".i") || util::hasSuffix(filename, ".s");
}

std::vector<std::filesystem::path> listCompilerTemps(const std::filesystem::path& root) {
    std::vector<std::filesystem::path> temps;
    if (!std::filesystem::exists(root)) {
        return temps;
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (isCompilerTempName(entry.path().filename().string())) {
            temps.push_back(entry.path());
        }
    }
    return temps;
}

} // namespace

TEST(Driver, frontendFailureDoesNotLeaveTempIntermediates) {
    IsolatedTmpDir tmp;
    auto sourcePath = writeTempSource("syntax_fail.c", "int main( {\n");
    ArgvBuffer args { { sourcePath.string() }, { "-c" } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("Error:"));
    const auto temps = listCompilerTemps(tmp.path());
    EXPECT_TRUE(temps.empty()) << "leaked temps:" << [&] {
        std::ostringstream out;
        for (const auto& t : temps) {
            out << " " << t;
        }
        return out.str();
    }();
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, successfulCompileDoesNotLeaveTempIntermediates) {
    IsolatedTmpDir tmp;
    auto sourcePath = writeTempSource("clean_temps.c", kTrivialMain);
    ArgvBuffer args { { sourcePath.string() }, { "-c" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    const auto temps = listCompilerTemps(tmp.path());
    EXPECT_TRUE(temps.empty()) << "leaked temps:" << [&] {
        std::ostringstream out;
        for (const auto& t : temps) {
            out << " " << t;
        }
        return out.str();
    }();
    removeCompileArtifacts(sourcePath);
}

TEST(Configuration, setStopAfterNeverDemotes) {
    Configuration configuration;
    configuration.setStopAfter(StopAfter::Object);
    configuration.setStopAfter(StopAfter::Link);
    EXPECT_EQ(configuration.stopAfter(), StopAfter::Object);
    configuration.setStopAfter(StopAfter::Assembly);
    EXPECT_EQ(configuration.stopAfter(), StopAfter::Assembly);
    configuration.setStopAfter(StopAfter::Object);
    EXPECT_EQ(configuration.stopAfter(), StopAfter::Assembly);
    configuration.setStopAfter(StopAfter::Preprocess);
    EXPECT_EQ(configuration.stopAfter(), StopAfter::Preprocess);
    configuration.setStopAfter(StopAfter::Assembly);
    EXPECT_EQ(configuration.stopAfter(), StopAfter::Preprocess);
}

TEST(Configuration, stageSettersAreActionsNotToggles) {
    Configuration configuration;
    configuration.setCompileOnly();
    EXPECT_EQ(configuration.stopAfter(), StopAfter::Object);
    EXPECT_TRUE(configuration.isCompileOnly());
    EXPECT_TRUE(configuration.stopsBeforeLink());

    configuration.setAssemblyOnly();
    EXPECT_EQ(configuration.stopAfter(), StopAfter::Assembly);
    EXPECT_TRUE(configuration.isAssemblyOnly());
    EXPECT_FALSE(configuration.isCompileOnly());

    configuration.setPreprocessOnly();
    EXPECT_EQ(configuration.stopAfter(), StopAfter::Preprocess);
    EXPECT_TRUE(configuration.isPreprocessOnly());
    EXPECT_FALSE(configuration.isAssemblyOnly());
}

TEST(Driver, mixedSourceAndAssemblyLinksWithDashO) {
    auto cPath = writeTempSource("mix_src.c", kTrivialMain);
    auto stem = cPath;
    stem.replace_extension();
    auto sPath = stem.string() + "_hand.s";
    ArgvBuffer asmArgs { { cPath.string() }, { "-S", "-o", sPath } };
    std::string errors;
    ASSERT_EQ(runDriver(asmArgs, &errors), 0) << errors;

    auto secondC = writeTempSource("mix_src2.c",
            "int other(void) { return 1; }\n");
    auto outPath = cPath.parent_path() / "mix_src_asm.out";
    std::filesystem::remove(outPath);
    ArgvBuffer linkArgs { { secondC.string(), sPath }, { "-o", outPath.string() } };
    EXPECT_EQ(runDriver(linkArgs, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(outPath));
    std::filesystem::remove(outPath);
    std::filesystem::remove(sPath);
    removeCompileArtifacts(cPath);
    removeCompileArtifacts(secondC);
}

TEST(Driver, preprocessedInputIsCompiledNotAssembled) {
    auto sourcePath = writeTempSource("kind_i.c", kTrivialMain);
    auto stem = sourcePath;
    stem.replace_extension();
    auto iPath = stem.string() + ".i";
    ArgvBuffer preprocessArgs { { sourcePath.string() }, { "-E", "-o", iPath } };
    std::string errors;
    ASSERT_EQ(runDriver(preprocessArgs, &errors), 0) << errors;
    auto objectPath = stem.string() + "_from_i.o";
    std::filesystem::remove(objectPath);
    ArgvBuffer compileArgs { { iPath }, { "-c", "-o", objectPath } };
    EXPECT_EQ(runDriver(compileArgs, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(objectPath));
    std::filesystem::remove(iPath);
    std::filesystem::remove(objectPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Compiler, assembleFileProducesObjectBesideAssembly) {
    auto sourcePath = writeTempSource("static_asm.c", kTrivialMain);
    auto stem = sourcePath;
    stem.replace_extension();
    auto sPath = stem.string() + ".s";
    ArgvBuffer asmArgs { { sourcePath.string() }, { "-S", "-o", sPath } };
    std::string errors;
    ASSERT_EQ(runDriver(asmArgs, &errors), 0) << errors;

    Configuration configuration = testConfiguration();
    configuration.setCompileOnly();
    auto objectPath = stem.string() + "_static.o";
    std::filesystem::remove(objectPath);
    EXPECT_EQ(Compiler::assembleFile(sPath, configuration), sPath + ".o");
    EXPECT_TRUE(std::filesystem::exists(sPath + ".o"));
    std::filesystem::remove(sPath + ".o");
    configuration.setOutputPath(objectPath);
    EXPECT_EQ(Compiler::assembleFile(sPath, configuration), objectPath);
    EXPECT_TRUE(std::filesystem::exists(objectPath));
    std::filesystem::remove(objectPath);
    std::filesystem::remove(sPath);
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

TEST(Compiler, preprocessCommandForwardsMmdWithIncludes) {
    Configuration configuration;
    configuration.setPreprocessorArgs({
            "-I", "inc", "-MMD", "-MP", "-MF", "a.d", "-MQ", "a.o"
    });
    auto argv = Compiler::preprocessCommand("a.c", "a.i", configuration);
    EXPECT_THAT(argv, ElementsAre(
            "gcc", "-E", "-x", "c", "-I", "inc",
            "-MMD", "-MP", "-MF", "a.d", "-MQ", "a.o", "-o", "a.i", "a.c"));
}

TEST(Compiler, linkCommandIsObjectsThenLinkerArgs) {
    auto argv = Compiler::linkCommand({ "a.o" }, "a.out", {});
    EXPECT_THAT(argv, ElementsAre("gcc", "-m64", "-pie", "-o", "a.out", "a.o"));
}

TEST(Compiler, linkCommandAppendsLinkerArgsAfterObjects) {
    auto argv = Compiler::linkCommand(
            { "a.o", "b.o" }, "prog", { "-L.", "-lm", "-pthread", "-Wl,-as-needed" });
    EXPECT_THAT(argv, ElementsAre(
            "gcc", "-m64", "-pie", "-o", "prog", "a.o", "b.o",
            "-L.", "-lm", "-pthread", "-Wl,-as-needed"));
}

TEST(Driver, linksWithLibm) {
    auto sourcePath = writeTempSource("link_libm.c",
            "double sqrt(double);\n"
            "int main(void) {\n"
            "    return sqrt(4.0) == 2.0 ? 0 : 1;\n"
            "}\n");
    auto outPath = sourcePath.parent_path() / "link_libm.out";
    std::filesystem::remove(outPath);
    ArgvBuffer args { { sourcePath.string() }, { "-lm", "-o", outPath.string() } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(outPath));
    EXPECT_EQ(util::runProcess({ outPath.string() }).exitCode, 0);
    std::filesystem::remove(outPath);
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, compileOnlyWithLinkerFlagsDoesNotLink) {
    auto sourcePath = writeTempSource("c_with_lm.c", kTrivialMain);
    ArgvBuffer args { { sourcePath.string() }, { "-c", "-lm", "-pthread" } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(std::filesystem::exists(sourcePath.string() + ".o"));
    EXPECT_FALSE(std::filesystem::exists(sourcePath.string() + ".out"));
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, returnsNonZeroWhenCompileOnlyWithArchive) {
    auto archive = writeTempSource("libdummy.a", "!<arch>\n");
    ArgvBuffer args { { archive.string() }, { "-c" } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("-c cannot be used with object files"));
    std::filesystem::remove(archive);
}

TEST(Driver, linksObjectWithArchiveWithoutCompilingArchive) {
    auto dir = std::filesystem::temp_directory_path() / "trans_driver_exit_tests";
    std::filesystem::create_directories(dir);
    auto addSrc = dir / "archive_add.c";
    auto mainSrc = dir / "archive_main.c";
    auto addObj = dir / "archive_add.o";
    auto archive = dir / "libadd.a";
    auto mainObj = dir / "archive_main.o";
    auto exe = dir / "archive_link.out";
    {
        std::ofstream out { addSrc };
        out << "int add(int a, int b) { return a + b; }\n";
    }
    {
        std::ofstream out { mainSrc };
        out << "int add(int a, int b);\n"
               "int main(void) { return add(40, 2) == 42 ? 0 : 1; }\n";
    }
    ASSERT_EQ(util::runProcess({ "gcc", "-c", "-fPIE", "-o", addObj.string(), addSrc.string() }).exitCode, 0);
    ASSERT_EQ(util::runProcess({ "ar", "rcs", archive.string(), addObj.string() }).exitCode, 0);

    std::string errors;
    ArgvBuffer compileMain { { mainSrc.string() }, { "-c", "-o", mainObj.string() } };
    ASSERT_EQ(runDriver(compileMain, &errors), 0) << errors;

    std::string output;
    ArgvBuffer linkArgs { { mainObj.string(), archive.string() }, { "-o", exe.string() } };
    EXPECT_EQ(runDriver(linkArgs, &errors, &output), 0) << errors;
    EXPECT_THAT(output, Not(HasSubstr("Compiling")));
    EXPECT_TRUE(std::filesystem::exists(exe));
    EXPECT_EQ(util::runProcess({ exe.string() }).exitCode, 0);

    std::filesystem::remove(addSrc);
    std::filesystem::remove(mainSrc);
    std::filesystem::remove(addObj);
    std::filesystem::remove(archive);
    std::filesystem::remove(mainObj);
    std::filesystem::remove(exe);
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
