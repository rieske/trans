#include "TestFixtures.h"

#include "util/Process.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>

namespace {

// Host link should produce a position-independent executable (PIE), not a
// fixed-address ET_EXEC that only works with gcc -no-pie.
bool executableIsPie(const std::string& path) {
    util::ProcessResult result = util::runProcess({ "readelf", "-h", path });
    if (result.exitCode != 0) {
        return false;
    }
    // readelf -h: "Type: DYN (Position-Independent Executable file)" on PIE.
    // Non-PIE linked with -no-pie is "Type: EXEC (Executable file)".
    return result.stdoutOutput.find("DYN") != std::string::npos
            && result.stdoutOutput.find("Position-Independent") != std::string::npos;
}

TEST(Compiler, linkedExecutableIsPositionIndependent) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", 42);
            return 0;
        }
    )prg"};

    program.compile();
    // Program::executableFile is source path + ".out" (see TestFixtures).
    const std::string executable = program.getSourceFilePath() + ".out";
    ASSERT_TRUE(executableIsPie(executable))
            << "expected PIE (readelf Type DYN / Position-Independent); "
               "got non-PIE executable (often forced by gcc -no-pie)";
    program.runAndExpect("42");
}

TEST(Compiler, pieExecutableCallsLibcAndLocalFunctionPointer) {
    // Direct libc call (PLT) plus address-of a same-TU function (lea [rel ...]).
    SourceProgram program{R"prg(
        int seven() {
            return 7;
        }

        int main() {
            int (*fp)();
            fp = seven;
            printf("%d", fp());
            return 0;
        }
    )prg"};

    program.compile();
    const std::string executable = program.getSourceFilePath() + ".out";
    ASSERT_TRUE(executableIsPie(executable))
            << "expected PIE executable for mixed local function pointer + printf";
    program.runAndExpect("7");
}

} // namespace
