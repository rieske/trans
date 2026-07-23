#include "TestFixtures.h"

#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"
#include "util/LogManager.h"
#include "ResourceHelpers.h"

#include <fstream>
#include <iterator>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace {

bool fileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

std::string writeTmpSource(const std::string& name, const std::string& body) {
    ensureTestProgramsTmpDir();
    std::string path = getTestProgramsTmpDir() + name;
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Could not write " + path);
    }
    out << body;
    return path;
}

std::string readFile(const std::string& path) {
    std::ifstream in(path);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

int runDriver(const std::vector<std::string>& arguments, std::string* errOut = nullptr) {
    std::vector<std::string> args = arguments;
    std::vector<char*> argv;
    for (auto& s : args) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    if (errOut) {
        *errOut = err.str();
    }
    return status;
}

// Two translation units: define in one, call from the other, single linked binary.
TEST(MultiTu, defineInOneTuCallFromAnother) {
    std::string libSrc = writeTmpSource("multi_tu_lib.src", R"prg(
        int add_one(int x) {
            return x + 1;
        }
    )prg");
    std::string mainSrc = writeTmpSource("multi_tu_main.src", R"prg(
        int add_one(int x);
        int main() {
            printf("%d", add_one(41));
            return 0;
        }
    )prg");

    std::string libObj = libSrc + ".o";
    std::string mainObj = mainSrc + ".o";
    std::string exe = getTestProgramsTmpDir() + "multi_tu_linked.out";
    std::string outputFile = exe + ".execution.output";
    remove(libObj.c_str());
    remove(mainObj.c_str());
    remove(exe.c_str());
    remove(outputFile.c_str());

    std::string err;
    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "--no-preprocess", "-c", "-o", libObj, libSrc
    }, &err), 0) << err;
    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "--no-preprocess", "-c", "-o", mainObj, mainSrc
    }, &err), 0) << err;
    ASSERT_TRUE(fileExists(libObj));
    ASSERT_TRUE(fileExists(mainObj));

    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "-o", exe, mainObj, libObj
    }, &err), 0) << err;
    ASSERT_TRUE(fileExists(exe));

    ASSERT_EQ(system((exe + " > " + outputFile).c_str()), 0);
    EXPECT_THAT(readFile(outputFile), Eq("42"));
}

// static function in one TU must not satisfy a call from another TU (link failure).
TEST(MultiTu, staticDoesNotExportAcrossTu) {
    std::string libSrc = writeTmpSource("multi_tu_static_lib.src", R"prg(
        static int hidden(void) {
            return 7;
        }
    )prg");
    std::string mainSrc = writeTmpSource("multi_tu_static_main.src", R"prg(
        int hidden(void);
        int main() {
            printf("%d", hidden());
            return 0;
        }
    )prg");

    std::string libObj = libSrc + ".o";
    std::string mainObj = mainSrc + ".o";
    std::string exe = getTestProgramsTmpDir() + "multi_tu_static.out";
    remove(libObj.c_str());
    remove(mainObj.c_str());
    remove(exe.c_str());

    std::string err;
    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "--no-preprocess", "-c", "-o", libObj, libSrc
    }, &err), 0) << err;
    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "--no-preprocess", "-c", "-o", mainObj, mainSrc
    }, &err), 0) << err;

    int status = runDriver({
            "trans", "-r../../../", "-o", exe, mainObj, libObj
    }, &err);
    // Host linker must fail to resolve `hidden` (static in the other TU).
    EXPECT_NE(status, 0) << err;
}

// Shared extern data: definition in one TU, use in another.
TEST(MultiTu, externDataDefinedInOneTu) {
    std::string dataSrc = writeTmpSource("multi_tu_data.src", R"prg(
        int shared_counter;
        void bump(void) {
            shared_counter = shared_counter + 1;
        }
    )prg");
    std::string mainSrc = writeTmpSource("multi_tu_data_main.src", R"prg(
        extern int shared_counter;
        void bump(void);
        int main() {
            shared_counter = 10;
            bump();
            bump();
            printf("%d", shared_counter);
            return 0;
        }
    )prg");

    std::string dataObj = dataSrc + ".o";
    std::string mainObj = mainSrc + ".o";
    std::string exe = getTestProgramsTmpDir() + "multi_tu_data.out";
    std::string outputFile = exe + ".execution.output";
    remove(dataObj.c_str());
    remove(mainObj.c_str());
    remove(exe.c_str());
    remove(outputFile.c_str());

    std::string err;
    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "--no-preprocess", "-c", "-o", dataObj, dataSrc
    }, &err), 0) << err;
    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "--no-preprocess", "-c", "-o", mainObj, mainSrc
    }, &err), 0) << err;
    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "-o", exe, mainObj, dataObj
    }, &err), 0) << err;

    ASSERT_EQ(system((exe + " > " + outputFile).c_str()), 0);
    EXPECT_THAT(readFile(outputFile), Eq("12"));
}

// Two TUs each define the same global symbol: host linker must fail (multiple definition).
TEST(MultiTu, duplicateGlobalDefinitionFailsToLink) {
    std::string aSrc = writeTmpSource("multi_tu_dup_a.src", R"prg(
        int shared_counter;
        int main() {
            shared_counter = 1;
            printf("%d", shared_counter);
            return 0;
        }
    )prg");
    std::string bSrc = writeTmpSource("multi_tu_dup_b.src", R"prg(
        int shared_counter;
        void bump(void) {
            shared_counter = shared_counter + 1;
        }
    )prg");

    std::string aObj = aSrc + ".o";
    std::string bObj = bSrc + ".o";
    std::string exe = getTestProgramsTmpDir() + "multi_tu_dup.out";
    remove(aObj.c_str());
    remove(bObj.c_str());
    remove(exe.c_str());

    std::string err;
    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "--no-preprocess", "-c", "-o", aObj, aSrc
    }, &err), 0) << err;
    ASSERT_EQ(runDriver({
            "trans", "-r../../../", "--no-preprocess", "-c", "-o", bObj, bSrc
    }, &err), 0) << err;

    int status = runDriver({
            "trans", "-r../../../", "-o", exe, aObj, bObj
    }, &err);
    EXPECT_NE(status, 0) << err;
}

} // namespace
