#include "TestFixtures.h"

#include "util/Process.h"
#include "DriverHarness.h"
#include "NmSymbols.h"
#include "ResourceHelpers.h"

#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace {

bool fileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

std::string dialectStem(const std::string& base) {
    return base + "_" + functionalTestDialectTag();
}

std::string writeTmpC(const std::string& stem, const std::string& body) {
    return writeTempSource(dialectStem(stem), body);
}

std::string readFile(const std::string& path) {
    std::ifstream in { path };
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void removePath(const std::string& path) {
    unlink(path.c_str());
}

std::vector<std::string> dialectFlags(std::vector<std::string> flags) {
    flags.insert(flags.begin(), "-a" + functionalTestDialectTag());
    return flags;
}

int compileOnly(const std::string& sourcePath, std::string* errOut = nullptr) {
    ArgvBuffer args { { sourcePath }, dialectFlags({ "-c" }) };
    return runDriver(args, errOut);
}

int compileOnlyTo(const std::string& sourcePath, const std::string& objectPath,
        std::string* errOut = nullptr) {
    ArgvBuffer args { { sourcePath }, dialectFlags({ "-c", "-o" + objectPath }) };
    return runDriver(args, errOut);
}

// Host linker: isolates object ABI from Driver link-only. gcc is already the
// product linker; this checks that trans .o files are usable as inputs.
int hostLink(const std::vector<std::string>& objects, const std::string& exe) {
    std::vector<std::string> argv { "gcc", "-m64", "-pie", "-o", exe };
    argv.insert(argv.end(), objects.begin(), objects.end());
    return util::runProcess(argv).exitCode;
}

int runExe(const std::string& exe, const std::string& outputFile) {
    removePath(outputFile);
    return util::runProcess({ exe }, {}, outputFile).exitCode;
}

std::string nmObject(const std::string& objectPath) {
    util::ProcessResult result = util::runProcess({ "nm", "-P", objectPath });
    EXPECT_EQ(result.exitCode, 0) << result.stderrOutput;
    return result.stdoutOutput;
}

int runTransBinary(const std::vector<std::string>& extraAndFiles, std::string* errOut = nullptr) {
    std::vector<std::string> argv {
            transBinaryPath(),
            "-r" + getResourcesBaseDir(),
            "-a" + functionalTestDialectTag()
    };
    argv.insert(argv.end(), extraAndFiles.begin(), extraAndFiles.end());
    util::ProcessResult result = util::runProcess(argv);
    if (errOut) {
        *errOut = result.stderrOutput;
    }
    return result.exitCode;
}

TEST(MultiTu, nmExportsNonStaticFunctionAndObject) {
    std::string src = writeTmpC("multi_tu_export", R"prg(
        int g;
        int foo(void) {
            return 1;
        }
        int main(void) {
            return foo() + g;
        }
    )prg");
    std::string obj = src + ".o";
    removePath(obj);

    std::string err;
    ASSERT_EQ(compileOnly(src, &err), 0) << err;
    ASSERT_TRUE(fileExists(obj));

    const std::string nm = nmObject(obj);
    EXPECT_TRUE(nmTypeIsGlobal(nmSymbolType(nm, "foo"))) << nm;
    EXPECT_TRUE(nmTypeIsGlobal(nmSymbolType(nm, "g"))) << nm;
    EXPECT_TRUE(nmTypeIsGlobal(nmSymbolType(nm, "main"))) << nm;
}

TEST(MultiTu, nmDoesNotExportStaticFunctionOrObject) {
    std::string src = writeTmpC("multi_tu_static_nm", R"prg(int printf(const char *, ...);
        static int g;
        static int hidden(void) {
            return 1;
        }
        int main(void) {
            g = g + hidden();
            printf("%d", g);
            return 0;
        }
    )prg");
    std::string obj = src + ".o";
    removePath(obj);

    std::string err;
    ASSERT_EQ(compileOnly(src, &err), 0) << err;
    const std::string nm = nmObject(obj);
    EXPECT_FALSE(nmTypeIsGlobal(nmSymbolType(nm, "hidden"))) << nm;
    EXPECT_FALSE(nmTypeIsGlobal(nmSymbolType(nm, "g"))) << nm;

    SourceProgram program { readFile(src) };
    program.compile();
    program.runAndExpect("1");
}

TEST(MultiTu, defineInOneTuCallFromAnother) {
    std::string libSrc = writeTmpC("multi_tu_lib", R"prg(
        int add_one(int x) {
            return x + 1;
        }
    )prg");
    std::string mainSrc = writeTmpC("multi_tu_main", R"prg(int printf(const char *, ...);
        int add_one(int x);
        int main(void) {
            printf("%d", add_one(41));
            return 0;
        }
    )prg");
    std::string libObj = libSrc + ".o";
    std::string mainObj = mainSrc + ".o";
    std::string exe = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_linked") + ".out";
    std::string outputFile = exe + ".execution.output";
    removePath(libObj);
    removePath(mainObj);
    removePath(exe);
    removePath(outputFile);

    std::string err;
    ASSERT_EQ(compileOnly(libSrc, &err), 0) << err;
    ASSERT_EQ(compileOnly(mainSrc, &err), 0) << err;
    ASSERT_EQ(hostLink({ mainObj, libObj }, exe), 0);
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq("42"));
}

TEST(MultiTu, externDataDefinedInOneTu) {
    std::string dataSrc = writeTmpC("multi_tu_data", R"prg(
        int shared_counter;
        void bump(void) {
            shared_counter = shared_counter + 1;
        }
    )prg");
    std::string mainSrc = writeTmpC("multi_tu_data_main", R"prg(int printf(const char *, ...);
        extern int shared_counter;
        void bump(void);
        int main(void) {
            shared_counter = 10;
            bump();
            bump();
            printf("%d", shared_counter);
            return 0;
        }
    )prg");
    std::string dataObj = dataSrc + ".o";
    std::string mainObj = mainSrc + ".o";
    std::string exe = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_data") + ".out";
    std::string outputFile = exe + ".execution.output";
    removePath(dataObj);
    removePath(mainObj);
    removePath(exe);
    removePath(outputFile);

    std::string err;
    ASSERT_EQ(compileOnly(dataSrc, &err), 0) << err;
    ASSERT_EQ(compileOnly(mainSrc, &err), 0) << err;
    ASSERT_EQ(hostLink({ mainObj, dataObj }, exe), 0);
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq("12"));
}

TEST(MultiTu, staticDoesNotSatisfyOtherTu) {
    std::string libSrc = writeTmpC("multi_tu_hidden_lib", R"prg(
        static int hidden(void) {
            return 7;
        }
    )prg");
    std::string mainSrc = writeTmpC("multi_tu_hidden_main", R"prg(int printf(const char *, ...);
        int hidden(void);
        int main(void) {
            printf("%d", hidden());
            return 0;
        }
    )prg");
    std::string libObj = libSrc + ".o";
    std::string mainObj = mainSrc + ".o";
    std::string exe = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_hidden") + ".out";
    removePath(libObj);
    removePath(mainObj);
    removePath(exe);

    std::string err;
    ASSERT_EQ(compileOnly(libSrc, &err), 0) << err;
    ASSERT_EQ(compileOnly(mainSrc, &err), 0) << err;
    EXPECT_NE(hostLink({ mainObj, libObj }, exe), 0);
}

TEST(MultiTu, twoTusEachWithStaticSameName) {
    std::string aSrc = writeTmpC("multi_tu_static_a", R"prg(int printf(const char *, ...);
        static int x;
        int get_a(void) {
            x = x + 1;
            return x;
        }
        int get_b(void);
        int main(void) {
            printf("%d %d %d %d", get_a(), get_a(), get_b(), get_b());
            return 0;
        }
    )prg");
    std::string bSrc = writeTmpC("multi_tu_static_b", R"prg(
        static int x;
        int get_b(void) {
            x = x + 1;
            return x;
        }
    )prg");
    std::string aObj = aSrc + ".o";
    std::string bObj = bSrc + ".o";
    std::string exe = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_static_same") + ".out";
    std::string outputFile = exe + ".execution.output";
    removePath(aObj);
    removePath(bObj);
    removePath(exe);
    removePath(outputFile);

    std::string err;
    ASSERT_EQ(compileOnly(aSrc, &err), 0) << err;
    ASSERT_EQ(compileOnly(bSrc, &err), 0) << err;
    ASSERT_EQ(hostLink({ aObj, bObj }, exe), 0);
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq("1 2 1 2"));
}

TEST(MultiTu, gccObjectLinksWithTransMain) {
    std::string addSrc = writeTmpC("multi_tu_gcc_add", R"prg(
        int add(int a, int b) {
            return a + b;
        }
    )prg");
    std::string mainSrc = writeTmpC("multi_tu_gcc_main", R"prg(int printf(const char *, ...);
        int add(int a, int b);
        int main(void) {
            printf("%d", add(20, 22));
            return 0;
        }
    )prg");
    std::string addObj = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_gcc_add") + ".o";
    std::string mainObj = mainSrc + ".o";
    std::string exe = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_gcc_mix") + ".out";
    std::string outputFile = exe + ".execution.output";
    removePath(addObj);
    removePath(mainObj);
    removePath(exe);
    removePath(outputFile);

    ASSERT_EQ(util::runProcess({ "gcc", "-c", "-fPIE", "-o", addObj, addSrc }).exitCode, 0);
    std::string err;
    ASSERT_EQ(compileOnly(mainSrc, &err), 0) << err;
    ASSERT_EQ(hostLink({ mainObj, addObj }, exe), 0);
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq("42"));
}

TEST(MultiTu, transObjectLinksWithGccMain) {
    std::string addSrc = writeTmpC("multi_tu_trans_add", R"prg(
        int add(int a, int b) {
            return a + b;
        }
    )prg");
    std::string mainSrc = writeTmpC("multi_tu_trans_main_gcc", R"prg(
        int printf(const char *, ...);
        int add(int a, int b);
        int main(void) {
            printf("%d", add(20, 22));
            return 0;
        }
    )prg");
    std::string addObj = addSrc + ".o";
    std::string mainObj = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_gcc_mainobj") + ".o";
    std::string exe = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_trans_mix") + ".out";
    std::string outputFile = exe + ".execution.output";
    removePath(addObj);
    removePath(mainObj);
    removePath(exe);
    removePath(outputFile);

    std::string err;
    ASSERT_EQ(compileOnly(addSrc, &err), 0) << err;
    ASSERT_EQ(util::runProcess({ "gcc", "-c", "-fPIE", "-o", mainObj, mainSrc }).exitCode, 0);
    ASSERT_EQ(hostLink({ mainObj, addObj }, exe), 0);
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq("42"));
}

TEST(MultiTu, transLinksTwoObjects) {
    std::string libSrc = writeTmpC("multi_tu_translink_lib", R"prg(
        int add_one(int x) {
            return x + 1;
        }
    )prg");
    std::string mainSrc = writeTmpC("multi_tu_translink_main", R"prg(int printf(const char *, ...);
        int add_one(int x);
        int main(void) {
            printf("%d", add_one(41));
            return 0;
        }
    )prg");
    std::string libObj = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_translink_lib") + ".o";
    std::string mainObj = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_translink_main") + ".o";
    std::string exe = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_translink") + ".out";
    std::string outputFile = exe + ".execution.output";
    removePath(libObj);
    removePath(mainObj);
    removePath(exe);
    removePath(outputFile);

    std::string err;
    ASSERT_EQ(compileOnlyTo(libSrc, libObj, &err), 0) << err;
    ASSERT_EQ(compileOnlyTo(mainSrc, mainObj, &err), 0) << err;

    // Product CLI: invoke the binary so a parser exit() cannot kill this test process.
    ASSERT_EQ(runTransBinary({ "-o" + exe, mainObj, libObj }, &err), 0) << err;
    ASSERT_TRUE(fileExists(exe));
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq("42"));
}

TEST(MultiTu, transCompilesTwoSourcesWithDashO) {
    std::string libSrc = writeTmpC("multi_tu_csrc_lib", R"prg(
        int add_one(int x) {
            return x + 1;
        }
    )prg");
    std::string mainSrc = writeTmpC("multi_tu_csrc_main", R"prg(int printf(const char *, ...);
        int add_one(int x);
        int main(void) {
            printf("%d", add_one(41));
            return 0;
        }
    )prg");
    std::string exe = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_csrc") + ".out";
    std::string outputFile = exe + ".execution.output";
    removePath(exe);
    removePath(outputFile);

    std::string err;
    ASSERT_EQ(runTransBinary({ "-o" + exe, mainSrc, libSrc }, &err), 0) << err;
    ASSERT_TRUE(fileExists(exe));
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq("42"));
}

TEST(MultiTu, transCompilesTwoSourcesWithExternData) {
    std::string dataSrc = writeTmpC("multi_tu_csrc_data", R"prg(
        int shared_counter;
        void bump(void) {
            shared_counter = shared_counter + 1;
        }
    )prg");
    std::string mainSrc = writeTmpC("multi_tu_csrc_data_main", R"prg(int printf(const char *, ...);
        extern int shared_counter;
        void bump(void);
        int main(void) {
            shared_counter = 10;
            bump();
            bump();
            printf("%d", shared_counter);
            return 0;
        }
    )prg");
    std::string exe = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_csrc_data") + ".out";
    std::string outputFile = exe + ".execution.output";
    removePath(exe);
    removePath(outputFile);

    std::string err;
    ASSERT_EQ(runTransBinary({ "-o" + exe, mainSrc, dataSrc }, &err), 0) << err;
    ASSERT_EQ(runExe(exe, outputFile), 0);
    EXPECT_THAT(readFile(outputFile), Eq("12"));
}

TEST(MultiTu, transTwoSourcesWithoutDashOIsError) {
    std::string aSrc = writeTmpC("multi_tu_no_o_a", R"prg(
        int main(void) { return 0; }
    )prg");
    std::string bSrc = writeTmpC("multi_tu_no_o_b", R"prg(
        int main(void) { return 0; }
    )prg");
    removePath(aSrc + ".out");
    removePath(bSrc + ".out");
    std::string err;
    EXPECT_NE(runTransBinary({ aSrc, bSrc }, &err), 0);
    EXPECT_THAT(err, HasSubstr("requires -o"));
    EXPECT_FALSE(fileExists(aSrc + ".out"));
    EXPECT_FALSE(fileExists(bSrc + ".out"));
}

TEST(MultiTu, transCompileOnlyDashOWithTwoSourcesIsError) {
    std::string aSrc = writeTmpC("multi_tu_c_two_a", R"prg(
        int a(void) { return 1; }
    )prg");
    std::string bSrc = writeTmpC("multi_tu_c_two_b", R"prg(
        int b(void) { return 2; }
    )prg");
    std::string obj = getTestResourcePath("programs/tmp/") + dialectStem("multi_tu_c_two") + ".o";
    removePath(obj);

    std::string err;
    EXPECT_NE(runTransBinary({ "-c", "-o" + obj, aSrc, bSrc }, &err), 0);
    EXPECT_FALSE(fileExists(obj));
}

TEST(MultiTu, transObjectWithoutDashOIsError) {
    std::string src = writeTmpC("multi_tu_obj_no_o", R"prg(
        int main(void) {
            return 0;
        }
    )prg");
    std::string obj = src + ".o";
    removePath(obj);
    std::string err;
    ASSERT_EQ(compileOnly(src, &err), 0) << err;

    EXPECT_NE(runTransBinary({ obj }, &err), 0);
}

} // namespace
