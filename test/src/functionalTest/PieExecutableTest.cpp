#include "TestFixtures.h"

#include "util/Process.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <sstream>
#include <string>

namespace {

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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d", 42);
            return 0;
        }
    )prg"};

    program.compile();
    ASSERT_TRUE(executableIsPie(program.getExecutablePath()))
            << "expected PIE (readelf Type DYN) for backend=" << functionalTestDialectTag()
            << "; compiler link must pass -pie";
    program.runAndExpect("42");
}

TEST(Compiler, pieExecutableCallsLibcAndLocalFunctionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    ASSERT_TRUE(executableIsPie(program.getExecutablePath()))
            << "expected PIE executable for backend=" << functionalTestDialectTag()
            << " (mixed local function pointer + printf)";
    program.runAndExpect("7");
}

TEST(Compiler, pieExecutableTakesExternFunctionAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    ASSERT_TRUE(executableIsPie(program.getExecutablePath()))
            << "expected PIE executable for backend=" << functionalTestDialectTag()
            << " (extern function address via GOT)";
    program.runAndExpect("1");
}

} // namespace
