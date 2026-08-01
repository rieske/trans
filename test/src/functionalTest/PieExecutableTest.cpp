#include "TestFixtures.h"

#include "util/Process.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <sstream>
#include <string>

namespace {

// Compiler product: link with -pie. ET_DYN for an executable output (not a .so we
// produce). Matches both modern readelf ("Position-Independent Executable") and
// older binutils ("Shared object file") wording on the Type line.
bool executableIsPie(const std::string& path) {
    util::ProcessResult result = util::runProcess({ "readelf", "-h", path });
    if (result.exitCode != 0) {
        return false;
    }
    std::istringstream in { result.stdoutOutput };
    std::string line;
    while (std::getline(in, line)) {
        if (line.find("Type:") != std::string::npos && line.find("DYN") != std::string::npos) {
            return true;
        }
    }
    return false;
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
            << "expected PIE (readelf Type DYN); compiler link must pass -pie";
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

TEST(Compiler, pieExecutableTakesExternFunctionAddress) {
    // Address-of libc (GOT load under PIE). Equal materializations prove assemble+link+run.
    // Call-through uses the pointer type (not external-varargs), so do not invoke via fp.
    SourceProgram program{R"prg(
        int main() {
            int (*a)();
            int (*b)();
            a = printf;
            b = printf;
            printf("%d", a == b);
            return 0;
        }
    )prg"};

    program.compile();
    const std::string executable = program.getSourceFilePath() + ".out";
    ASSERT_TRUE(executableIsPie(executable))
            << "expected PIE executable for extern function address via GOT";
    program.runAndExpect("1");
}

} // namespace
