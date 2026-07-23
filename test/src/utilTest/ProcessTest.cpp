#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "util/Process.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace testing;
using namespace util;

namespace {

TEST(Process, capturesStdoutOfSuccessfulCommand) {
    ProcessResult result = runProcess({ "/bin/echo", "hello" });
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_EQ(result.stdoutOutput, "hello\n");
    EXPECT_TRUE(result.stderrOutput.empty());
}

TEST(Process, nonZeroExitIsReported) {
    ProcessResult result = runProcess({ "/bin/false" });
    EXPECT_NE(result.exitCode, 0);
}

TEST(Process, runProcessOrThrowThrowsOnFailure) {
    EXPECT_THROW(runProcessOrThrow({ "/bin/false" }), std::runtime_error);
}

TEST(Process, emptyArgvThrows) {
    EXPECT_THROW(runProcess({}), std::invalid_argument);
}

TEST(Process, missingExecutableFailsWithoutHanging) {
    ProcessResult result = runProcess({ "/no/such/trans_process_binary_xyz" });
    EXPECT_NE(result.exitCode, 0);
    // execvp failure in the child is reported as 127.
    EXPECT_EQ(result.exitCode, 127);
}

TEST(Process, nasmFailureSurfacesStderrInException) {
    // Garbage input forces a non-zero exit; runProcessOrThrow must include stderr.
    const auto asmPath = std::filesystem::temp_directory_path() / "trans_process_bad.S";
    {
        std::ofstream out { asmPath };
        out << "this is not valid nasm assembly!!!\n";
    }
    ProcessResult failed = runProcess({ "nasm", "-O1", "-f", "elf64", asmPath.string() });
    EXPECT_NE(failed.exitCode, 0);
    EXPECT_FALSE(failed.stderrOutput.empty());

    try {
        runProcessOrThrow({ "nasm", "-O1", "-f", "elf64", asmPath.string() });
        FAIL() << "expected nasm to fail on garbage assembly";
    } catch (const std::runtime_error& error) {
        const std::string message { error.what() };
        EXPECT_THAT(message, HasSubstr("command failed"));
        EXPECT_THAT(message, HasSubstr("nasm"));
        // Do not match version-specific wording; only that stderr was folded in.
        EXPECT_THAT(message, HasSubstr(failed.stderrOutput.substr(0, std::min<size_t>(32, failed.stderrOutput.size()))));
    }
    std::filesystem::remove(asmPath);
    std::filesystem::remove(std::filesystem::path(asmPath).replace_extension(".o"));
}

TEST(Process, feedsStdinAndRedirectsStdoutToFile) {
    const auto outPath = std::filesystem::temp_directory_path() / "trans_process_test_out.txt";
    std::filesystem::remove(outPath);

    ProcessResult result = runProcess({ "/bin/cat" }, "piped-input\n", outPath.string());
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_TRUE(result.stdoutOutput.empty());

    std::ifstream in { outPath };
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contents, "piped-input\n");

    std::filesystem::remove(outPath);
}

TEST(Process, drainsStdoutAndStderrWithoutDeadlock) {
    // Child writes > pipe capacity to both streams; sequential drain would hang.
    // First dd → stdout; second → stderr via cat 1>&2.
    ProcessResult result = runProcess({
            "/bin/sh", "-c",
            "dd if=/dev/zero bs=200000 count=1 2>/dev/null; "
            "dd if=/dev/zero bs=200000 count=1 2>/dev/null | /bin/cat 1>&2"
    });
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_EQ(result.stdoutOutput.size(), 200000u);
    EXPECT_EQ(result.stderrOutput.size(), 200000u);
}

} // namespace
