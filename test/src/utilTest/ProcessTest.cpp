#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "util/Process.h"

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
    // Single nasm run: non-zero exit + stderr folded into the throw message.
    const auto asmPath = std::filesystem::temp_directory_path() / "trans_process_bad.S";
    {
        std::ofstream out { asmPath };
        out << "this is not valid nasm assembly!!!\n";
    }
    try {
        runProcessOrThrow({ "nasm", "-O1", "-f", "elf64", asmPath.string() });
        FAIL() << "expected nasm to fail on garbage assembly";
    } catch (const std::runtime_error& error) {
        const std::string message { error.what() };
        EXPECT_THAT(message, HasSubstr("command failed"));
        EXPECT_THAT(message, HasSubstr("nasm"));
        // Message is "command failed (...): argv\n<stderr>".
        EXPECT_THAT(message, HasSubstr("\n"));
    }
    std::filesystem::remove(asmPath);
    std::filesystem::remove(std::filesystem::path(asmPath).replace_extension(".o"));
}

TEST(Process, runProcessOrThrowIncludesStdoutWhenStderrEmpty) {
    try {
        runProcessOrThrow({ "/bin/sh", "-c", "echo only-on-stdout; exit 3" });
        FAIL() << "expected throw";
    } catch (const std::runtime_error& error) {
        EXPECT_THAT(std::string(error.what()), HasSubstr("command failed (3)"));
        EXPECT_THAT(std::string(error.what()), HasSubstr("only-on-stdout"));
    }
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

TEST(Process, stdoutRedirectToMissingDirectoryFails) {
    ProcessResult result = runProcess(
            { "/bin/echo", "x" }, {}, "/no/such/trans_dir/out.txt");
    EXPECT_EQ(result.exitCode, 126);
}

TEST(Process, drainsStdoutAndStderrWithoutDeadlock) {
    // Child writes > pipe capacity to both streams; sequential drain would hang.
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
