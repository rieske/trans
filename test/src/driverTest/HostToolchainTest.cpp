#include "driver/HostToolchain.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

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

// Test-only display join (product uses Argv + Process, not shell strings).
std::string quoteForDisplay(const std::string& s) {
    std::string quoted = "'";
    for (char c : s) {
        if (c == '\'') {
            quoted += "'\\''";
        } else {
            quoted += c;
        }
    }
    quoted += "'";
    return quoted;
}

TEST(HostToolchain, quoteForDisplayEscapesSingleQuotes) {
    EXPECT_THAT(quoteForDisplay("a"), Eq("'a'"));
    EXPECT_THAT(quoteForDisplay("a'b"), Eq("'a'\\''b'"));
}

// Every fixed preprocess flag is intentional; Argv is the product surface.
TEST(HostToolchain, hostGccPreprocessArgvIncludeRequiredSwitches) {
    auto dialect = hostGccPreprocessDialectArgv();
    EXPECT_THAT(dialect, Contains("-E"));
    EXPECT_THAT(dialect, Contains("-P"));
    EXPECT_THAT(dialect, Contains("-std=c99"));
    EXPECT_THAT(dialect, Contains("-x"));
    EXPECT_THAT(dialect, Contains("c"));

    auto trailing = hostGccPreprocessTrailingArgv();
    EXPECT_THAT(trailing, Contains("-D__STDC__=0"));
    EXPECT_THAT(trailing, Contains("-DCURL_DISABLE_TYPECHECK"));
    EXPECT_THAT(trailing, Contains("-w"));
    // Ineffective alternative must not replace -w.
    EXPECT_THAT(trailing, Not(Contains("-Wno-builtin-macro-redefined")));
}

TEST(HostToolchain, fileExistsNonEmptyRequiresRegularNonEmptyFile) {
    std::string dir = makeTempDir("exists");
    std::string file = dir + "/f.o";
    EXPECT_FALSE(fileExistsNonEmpty(file));
    writeEmpty(file);
    EXPECT_FALSE(fileExistsNonEmpty(file));
    writeNonEmpty(file);
    EXPECT_TRUE(fileExistsNonEmpty(file));
    EXPECT_FALSE(fileExistsNonEmpty(dir)); // directory is not a non-empty regular file
    unlink(file.c_str());
    rmdir(dir.c_str());
}

TEST(HostToolchain, ensureNonEmptyObjectFileThrowsForMissingOrEmpty) {
    std::string dir = makeTempDir("empty_obj");
    std::string missing = dir + "/missing.o";
    std::string empty = dir + "/empty.o";
    writeEmpty(empty);

    try {
        ensureNonEmptyObjectFile(missing);
        FAIL() << "expected throw for missing object";
    } catch (const std::runtime_error& e) {
        EXPECT_THAT(std::string(e.what()), HasSubstr("empty object " + missing));
    }

    try {
        ensureNonEmptyObjectFile(empty);
        FAIL() << "expected throw for empty object";
    } catch (const std::runtime_error& e) {
        EXPECT_THAT(std::string(e.what()), HasSubstr("empty object " + empty));
    }

    std::string ok = dir + "/ok.o";
    writeNonEmpty(ok);
    EXPECT_NO_THROW(ensureNonEmptyObjectFile(ok));

    unlink(ok.c_str());
    unlink(empty.c_str());
    rmdir(dir.c_str());
}

TEST(HostToolchain, parentDirectoryStripsLastComponent) {
    EXPECT_THAT(parentDirectory("/a/b/c"), Eq("/a/b"));
    EXPECT_THAT(parentDirectory("/a"), Eq("/"));
    EXPECT_THAT(parentDirectory("/"), Eq(""));
    EXPECT_THAT(parentDirectory("rel"), Eq(""));
    EXPECT_THAT(parentDirectory("/a/b/"), Eq("/a"));
}

// findFileWalkingUp must see markers in ancestors, prefer nearer candidates,
// and honor require-non-empty vs empty-ok exists predicates.
TEST(HostToolchain, findFileWalkingUpFindsAncestorMarker) {
    std::string root = makeTempDir("walk_up");
    std::string mid = root + "/mid";
    std::string leaf = mid + "/leaf";
    mkdir(mid.c_str(), 0755);
    mkdir(leaf.c_str(), 0755);
    mkdir((root + "/resources").c_str(), 0755);
    mkdir((root + "/resources/configuration").c_str(), 0755);
    writeNonEmpty(root + "/resources/configuration/grammar.bnf");

    EXPECT_THAT(findFileWalkingUp(leaf, { "resources/configuration/grammar.bnf" }, 4, fileExists),
            Eq(root + "/resources/configuration/grammar.bnf"));
    EXPECT_THAT(findDirWalkingUp(leaf, "resources/configuration/grammar.bnf", 4, fileExists),
            Eq(root + "/"));

    // Empty file is found by fileExists but not fileExistsNonEmpty.
    writeEmpty(mid + "/empty.o");
    EXPECT_THAT(findFileWalkingUp(leaf, { "empty.o" }, 2, fileExists), Eq(mid + "/empty.o"));
    EXPECT_THAT(findFileWalkingUp(leaf, { "empty.o" }, 2, fileExistsNonEmpty), Eq(""));

    writeNonEmpty(mid + "/marker.o");
    writeNonEmpty(root + "/build_va.o");
    // Prefer first matching candidate at the nearest level.
    EXPECT_THAT(findFileWalkingUp(leaf, { "marker.o", "build_va.o" }, 3, fileExistsNonEmpty),
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

TEST(HostToolchain, findFileWalkingUpReturnsEmptyWhenMissing) {
    std::string root = makeTempDir("walk_miss");
    EXPECT_THAT(findFileWalkingUp(root, { "no_such_file.xyz" }, 3, fileExists), Eq(""));
    rmdir(root.c_str());
}

TEST(HostToolchain, buildHostLinkArgvIncludesPieAndLibs) {
    auto argv = buildHostLinkArgv({ "a.o", "b.o" }, "out", { "-lz", "-ldl" });
    ASSERT_FALSE(argv.empty());
    EXPECT_THAT(argv, Contains("gcc"));
    EXPECT_THAT(argv, Contains("-m64"));
    EXPECT_THAT(argv, Contains("-pie"));
    EXPECT_THAT(argv, Contains("-o"));
    EXPECT_THAT(argv, Contains("out"));
    EXPECT_THAT(argv, Contains("a.o"));
    EXPECT_THAT(argv, Contains("b.o"));
    EXPECT_THAT(argv, Contains("-lz"));
    EXPECT_THAT(argv, Contains("-ldl"));
    EXPECT_THAT(argv, Not(Contains("-lpthread")));
}

// Argv remains the product surface; display join is test-only.
TEST(HostToolchain, argvCanBeJoinedForDisplay) {
    auto argv = buildHostLinkArgv({ "a.o" }, "out", {});
    ASSERT_FALSE(argv.empty());
    std::string joined;
    for (const auto& a : argv) {
        if (!joined.empty()) {
            joined.push_back(' ');
        }
        joined += quoteForDisplay(a);
    }
    EXPECT_THAT(joined, HasSubstr(quoteForDisplay("gcc")));
    EXPECT_THAT(joined, HasSubstr(quoteForDisplay("a.o")));
}

} // namespace
