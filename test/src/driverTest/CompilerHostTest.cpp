#include "driver/Compiler.h"
#include "driver/Configuration.h"
#include "util/PathWalk.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace testing;

namespace {

std::string makeTempDir(const std::string& leaf) {
    std::string path = "/tmp/trans_host_" + leaf + "_" + std::to_string(getpid());
    mkdir(path.c_str(), 0755);
    return path;
}

void writeNonEmpty(const std::string& path) {
    std::ofstream out(path);
    out << "x";
}

void writeEmpty(const std::string& path) {
    std::ofstream out(path);
}

Configuration bareConfig() {
    return Configuration {};
}

TEST(Compiler, preprocessCommandOmitsExtraStdWhenUnset) {
    auto argv = Compiler::preprocessCommand("in.c", "in.i", bareConfig());
    EXPECT_THAT(argv, Contains("gcc"));
    EXPECT_THAT(argv, Contains("-E"));
    EXPECT_THAT(argv, Contains("-P"));
    EXPECT_THAT(argv, Contains("in.c"));
    EXPECT_THAT(argv, Contains("in.i"));
    // One -std=c99 from dialect; no second override.
    int stdCount = 0;
    for (const auto& a : argv) {
        if (a.rfind("-std=", 0) == 0) {
            ++stdCount;
        }
    }
    EXPECT_EQ(stdCount, 1);
}

TEST(Compiler, preprocessCommandAppendsPreprocessorStdFlag) {
    Configuration c;
    c.setPreprocessorStdFlag("c11");
    auto argv = Compiler::preprocessCommand("in.c", "in.i", c);
    EXPECT_THAT(argv, Contains("-std=c11"));
}

TEST(Compiler, preprocessCommandPlacesUserFlagsBeforeTrailing) {
    Configuration c;
    c.setPreprocessorArgs({ "-D", "USER=1" });
    auto argv = Compiler::preprocessCommand("in.c", "in.i", c);
    auto itUser = std::find(argv.begin(), argv.end(), "USER=1");
    auto itSt = std::find(argv.begin(), argv.end(), "-D__STDC__=0");
    ASSERT_NE(itUser, argv.end());
    ASSERT_NE(itSt, argv.end());
    EXPECT_LT(itUser, itSt);
}

TEST(Compiler, preprocessCommandAcceptsMultipleSources) {
    auto argv = Compiler::preprocessCommand(
            std::vector<std::string> { "a.c", "b.c" }, "out.i", bareConfig());
    EXPECT_THAT(argv, Contains("a.c"));
    EXPECT_THAT(argv, Contains("b.c"));
    EXPECT_THAT(argv, Contains("out.i"));
}

TEST(PathWalk, fileExistsNonEmptyRequiresRegularNonEmptyFile) {
    std::string dir = makeTempDir("exists");
    std::string file = dir + "/f";
    EXPECT_FALSE(util::fileExistsNonEmpty(file));
    writeEmpty(file);
    EXPECT_FALSE(util::fileExistsNonEmpty(file));
    writeNonEmpty(file);
    EXPECT_TRUE(util::fileExistsNonEmpty(file));
    EXPECT_FALSE(util::fileExistsNonEmpty(dir));
    unlink(file.c_str());
    rmdir(dir.c_str());
}

TEST(PathWalk, parentDirectoryStripsLastComponent) {
    EXPECT_THAT(util::parentDirectory("/a/b/c"), Eq("/a/b"));
    EXPECT_THAT(util::parentDirectory("/a"), Eq("/"));
    EXPECT_THAT(util::parentDirectory("rel"), Eq(""));
}

TEST(PathWalk, findFileWalkingUpFindsAncestorMarker) {
    std::string root = makeTempDir("walk");
    std::string mid = root + "/mid";
    std::string leaf = mid + "/leaf";
    mkdir(mid.c_str(), 0755);
    mkdir(leaf.c_str(), 0755);
    mkdir((root + "/resources").c_str(), 0755);
    mkdir((root + "/resources/configuration").c_str(), 0755);
    writeNonEmpty(root + "/resources/configuration/grammar.bnf");
    EXPECT_THAT(util::findFileWalkingUp(leaf, { "resources/configuration/grammar.bnf" }, 4, util::fileExists),
            Eq(root + "/resources/configuration/grammar.bnf"));
    EXPECT_THAT(util::findDirWalkingUp(leaf, "resources/configuration/grammar.bnf", 4, util::fileExists),
            Eq(root + "/"));
    writeEmpty(mid + "/empty.o");
    EXPECT_THAT(util::findFileWalkingUp(leaf, { "empty.o" }, 2, util::fileExists), Eq(mid + "/empty.o"));
    EXPECT_THAT(util::findFileWalkingUp(leaf, { "empty.o" }, 2, util::fileExistsNonEmpty), Eq(""));
    writeNonEmpty(mid + "/marker.o");
    writeNonEmpty(root + "/build_va.o");
    EXPECT_THAT(util::findFileWalkingUp(leaf, { "marker.o", "build_va.o" }, 3, util::fileExistsNonEmpty),
            Eq(mid + "/marker.o"));
    unlink((root + "/resources/configuration/grammar.bnf").c_str());
    rmdir((root + "/resources/configuration").c_str());
    rmdir((root + "/resources").c_str());
    unlink((mid + "/empty.o").c_str());
    unlink((mid + "/marker.o").c_str());
    unlink((root + "/build_va.o").c_str());
    rmdir(leaf.c_str());
    rmdir(mid.c_str());
    rmdir(root.c_str());
}

TEST(PathWalk, findFileWalkingUpReturnsEmptyWhenMissing) {
    std::string root = makeTempDir("walk_miss");
    EXPECT_THAT(util::findFileWalkingUp(root, { "no_such_file.xyz" }, 3, util::fileExists), Eq(""));
    rmdir(root.c_str());
}

} // namespace
