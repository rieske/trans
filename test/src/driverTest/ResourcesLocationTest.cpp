#include "driver/ResourcesLocation.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>

namespace {

using namespace testing;

std::filesystem::path makeGrammarAt(const std::filesystem::path& root) {
    const auto grammarDir = root / "resources" / "configuration";
    std::filesystem::create_directories(grammarDir);
    const auto grammar = grammarDir / "grammar.bnf";
    std::ofstream out { grammar };
    out << "<S> ::= 'x' ;\n";
    return grammar;
}

} // namespace

TEST(ResourcesLocation, usesDirectoryThatContainsResources) {
    const auto root = std::filesystem::temp_directory_path() / "trans_res_next_to_exe";
    std::filesystem::remove_all(root);
    makeGrammarAt(root);

    EXPECT_EQ(resourcesBaseFromExecutableDir(root), root.string() + "/");

    std::filesystem::remove_all(root);
}

TEST(ResourcesLocation, usesParentWhenBinaryLivesInBuildDir) {
    const auto root = std::filesystem::temp_directory_path() / "trans_res_build_parent";
    std::filesystem::remove_all(root);
    makeGrammarAt(root);
    const auto buildDir = root / "build";
    std::filesystem::create_directories(buildDir);

    EXPECT_EQ(resourcesBaseFromExecutableDir(buildDir), root.string() + "/");

    std::filesystem::remove_all(root);
}

TEST(ResourcesLocation, doesNotWalkPastParent) {
    const auto root = std::filesystem::temp_directory_path() / "trans_res_no_walk";
    std::filesystem::remove_all(root);
    makeGrammarAt(root);
    const auto nested = root / "build" / "test" / "src";
    std::filesystem::create_directories(nested);

    EXPECT_TRUE(resourcesBaseFromExecutableDir(nested).empty());

    std::filesystem::remove_all(root);
}
