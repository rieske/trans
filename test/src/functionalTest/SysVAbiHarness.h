#ifndef TEST_SYSV_ABI_HARNESS_H_
#define TEST_SYSV_ABI_HARNESS_H_

#include "TestFixtures.h"
#include "util/Process.h"
#include "DriverHarness.h"
#include "ResourceHelpers.h"

#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace sysv_abi {

enum class Compiler { Trans, Gcc };

inline std::string dialectStem(const std::string& base) {
    return base + "_" + functionalTestDialectTag();
}

inline std::string writeTmpC(const std::string& stem, const std::string& body) {
    return writeTempSource(dialectStem(stem), body);
}

inline std::string readFile(const std::string& path) {
    std::ifstream in { path };
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

inline void removePath(const std::string& path) {
    unlink(path.c_str());
}

inline std::vector<std::string> dialectFlags(std::vector<std::string> flags) {
    flags.insert(flags.begin(), "-masm=" + functionalTestDialectTag());
    return flags;
}

inline int compileTrans(const std::string& sourcePath, const std::string& objectPath, std::string* errOut) {
    ArgvBuffer args { { sourcePath }, dialectFlags({ "-c", "-o" + objectPath }) };
    return runDriver(args, errOut);
}

inline int compileGcc(const std::string& sourcePath, const std::string& objectPath, std::string* errOut) {
    // -O2 so gcc does not leave the other class's bits in rdx / on the
    // outgoing stack slot (O0 epilogues made mixed returns look correct).
    util::ProcessResult result = util::runProcess({
            "gcc", "-c", "-O2", "-fPIE", "-m64", "-o", objectPath, sourcePath
    });
    if (errOut) {
        *errOut = result.stderrOutput;
    }
    return result.exitCode;
}

inline int hostLink(const std::vector<std::string>& objects, const std::string& exe) {
    std::vector<std::string> argv { "gcc", "-m64", "-pie", "-o", exe };
    argv.insert(argv.end(), objects.begin(), objects.end());
    return util::runProcess(argv).exitCode;
}

inline int runExe(const std::string& exe, const std::string& outputFile) {
    removePath(outputFile);
    return util::runProcess({ exe }, {}, outputFile).exitCode;
}

inline void linkRunExpect(const std::string& stem, Compiler libCompiler, Compiler mainCompiler,
        const std::string& libBody, const std::string& mainBody, const std::string& expected) {
    const std::string libSrc = writeTmpC(stem + "_lib", libBody);
    const std::string mainSrc = writeTmpC(stem + "_main", mainBody);
    const std::string tmp = getTestResourcePath("programs/tmp/");
    const std::string libObj = tmp + dialectStem(stem + "_lib") + ".o";
    const std::string mainObj = tmp + dialectStem(stem + "_main") + ".o";
    const std::string exe = tmp + dialectStem(stem) + ".out";
    const std::string outputFile = exe + ".execution.output";
    removePath(libObj);
    removePath(mainObj);
    removePath(exe);
    removePath(outputFile);

    std::string err;
    const int libRc = libCompiler == Compiler::Gcc
            ? compileGcc(libSrc, libObj, &err)
            : compileTrans(libSrc, libObj, &err);
    ASSERT_EQ(libRc, 0) << err;
    err.clear();
    const int mainRc = mainCompiler == Compiler::Gcc
            ? compileGcc(mainSrc, mainObj, &err)
            : compileTrans(mainSrc, mainObj, &err);
    ASSERT_EQ(mainRc, 0) << err;
    ASSERT_EQ(hostLink({ mainObj, libObj }, exe), 0);
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq(expected));
}

} // namespace sysv_abi

#endif
