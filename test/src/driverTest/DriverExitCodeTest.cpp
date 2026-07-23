#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "driver/Compiler.h"
#include "driver/Configuration.h"
#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"
#include "util/LogManager.h"

#include "ResourceHelpers.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
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

// Restores PATH on destruction so failure paths cannot leave a poisoned environment.
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

// Installs a failing `nasm` ahead of the real one on PATH for the lifetime of this object.
class FakeFailingNasm {
public:
    FakeFailingNasm() {
        // Unique directory so concurrent test processes cannot clash.
        std::string templatePath =
                (std::filesystem::temp_directory_path() / "trans_fake_nasm_XXXXXX").string();
        std::vector<char> mutableTemplate(templatePath.begin(), templatePath.end());
        mutableTemplate.push_back('\0');
        char* created = ::mkdtemp(mutableTemplate.data());
        if (created == nullptr) {
            throw std::runtime_error("mkdtemp failed for FakeFailingNasm");
        }
        binDir_ = created;
        nasmPath_ = binDir_ / "nasm";
        {
            std::ofstream out { nasmPath_ };
            out << "#!/bin/sh\n"
                   "echo fake-nasm-failed >&2\n"
                   "exit 1\n";
        }
        std::filesystem::permissions(nasmPath_,
                std::filesystem::perms::owner_all
                        | std::filesystem::perms::group_exec
                        | std::filesystem::perms::others_exec);
        pathGuard_.emplace(binDir_.string());
    }

    ~FakeFailingNasm() {
        pathGuard_.reset();
        std::error_code ec;
        std::filesystem::remove_all(binDir_, ec);
    }

    FakeFailingNasm(const FakeFailingNasm&) = delete;
    FakeFailingNasm& operator=(const FakeFailingNasm&) = delete;

private:
    std::filesystem::path binDir_;
    std::filesystem::path nasmPath_;
    std::optional<PathGuard> pathGuard_;
};

// argv strings must outlive ConfigurationParser (it does not copy flags).
struct ArgvBuffer {
    std::string executable { "trans" };
    std::string resourcesFlag;
    std::vector<std::string> sources;
    std::vector<char*> pointers;

    explicit ArgvBuffer(std::vector<std::string> sourcePaths) :
            resourcesFlag { "-r" + getResourcesBaseDir() },
            sources { std::move(sourcePaths) } {
        pointers.push_back(executable.data());
        pointers.push_back(resourcesFlag.data());
        for (auto& source : sources) {
            pointers.push_back(source.data());
        }
    }

    int argc() const {
        return static_cast<int>(pointers.size());
    }

    char** argv() {
        return pointers.data();
    }
};

int runDriver(ArgvBuffer& args, std::string* errorOutput = nullptr) {
    std::stringstream outputStream;
    std::stringstream errorStream;
    int exitCode = 0;
    LogManager::withOutputStreams(outputStream, errorStream, [&]() {
        Driver driver {};
        exitCode = driver.run(ConfigurationParser { args.argc(), args.argv() });
    });
    if (errorOutput != nullptr) {
        *errorOutput = errorStream.str();
    }
    return exitCode;
}

Configuration testConfiguration() {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    return configuration;
}

const char* kTrivialMain =
        "int main() {\n"
        "    return 0;\n"
        "}\n";

TEST(Driver, returnsNonZeroWhenSourceIsMissing) {
    ArgvBuffer args { { "definitely_missing_source_file.src" } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("Error:"));
}

TEST(Driver, returnsZeroForSuccessfulCompile) {
    auto sourcePath = writeTempSource("ok_main.src", kTrivialMain);
    ArgvBuffer args { { sourcePath.string() } };
    std::string errors;
    EXPECT_EQ(runDriver(args, &errors), 0) << errors;
    EXPECT_TRUE(errors.empty());
    removeCompileArtifacts(sourcePath);
}

TEST(Driver, returnsNonZeroWhenAnySourceFailsInMultiFileRun) {
    auto goodPath = writeTempSource("multi_ok.src", kTrivialMain);
    ArgvBuffer args { { goodPath.string(), "definitely_missing_other.src" } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("Error:"));
    removeCompileArtifacts(goodPath);
}

TEST(Compiler, throwsWhenSourceCannotBeOpened) {
    Compiler compiler { testConfiguration() };
    EXPECT_THROW(compiler.compile("no_such_compile_input.src"), std::runtime_error);
}

TEST(Compiler, assembleFailureThrowsFromCompile) {
    auto sourcePath = writeTempSource("assemble_fail_compile.src", kTrivialMain);
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

TEST(Driver, assembleFailureReturnsNonZero) {
    auto sourcePath = writeTempSource("assemble_fail_driver.src", kTrivialMain);
    FakeFailingNasm fakeNasm;
    ArgvBuffer args { { sourcePath.string() } };
    std::string errors;
    EXPECT_NE(runDriver(args, &errors), 0);
    EXPECT_THAT(errors, HasSubstr("Error:"));
    removeCompileArtifacts(sourcePath);
}

} // namespace
