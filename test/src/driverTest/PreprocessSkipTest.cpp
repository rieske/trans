#include "driver/Compiler.h"
#include "driver/Configuration.h"

#include "gtest/gtest.h"

#include "ResourceHelpers.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

Configuration emptyConfig() {
    return Configuration {};
}

Configuration withPreprocessorArgs(std::vector<std::string> args) {
    Configuration configuration;
    configuration.setPreprocessorArgs(std::move(args));
    return configuration;
}

Configuration productConfig() {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    return configuration;
}

std::filesystem::path writeTemp(const std::string& name, const std::string& source) {
    auto dir = std::filesystem::temp_directory_path() / "trans_preprocess_skip_tests";
    std::filesystem::create_directories(dir);
    auto path = dir / name;
    std::ofstream out { path };
    out << source;
    return path;
}

bool needsGcc(const std::filesystem::path& path, const Configuration& configuration) {
    return Compiler::sourceFileNeedsGccPreprocessor(path.string(), configuration);
}

TEST(PreprocessSkip, sourceWithoutHashDoesNotNeedGcc) {
    auto path = writeTemp("no_hash.c", "int main(void) { return 0; }\n");
    EXPECT_FALSE(needsGcc(path, emptyConfig()));
    std::filesystem::remove(path);
}

TEST(PreprocessSkip, sourceWithIncludeNeedsGcc) {
    auto path = writeTemp("with_include.c", "#include <stdio.h>\nint main(){}\n");
    EXPECT_TRUE(needsGcc(path, emptyConfig()));
    std::filesystem::remove(path);
}

TEST(PreprocessSkip, sourceWithDefineNeedsGcc) {
    auto path = writeTemp("with_define.c", "#define X 1\nint main(){ return X; }\n");
    EXPECT_TRUE(needsGcc(path, emptyConfig()));
    std::filesystem::remove(path);
}

TEST(PreprocessSkip, hashInCommentStillNeedsGcc) {
    auto path = writeTemp("hash_comment.c", "// #define not real\nint main(){ return 0; }\n");
    EXPECT_TRUE(needsGcc(path, emptyConfig()));
    std::filesystem::remove(path);
}

TEST(PreprocessSkip, defineArgForcesGccWithoutHashInSource) {
    auto path = writeTemp("define_arg.c", "int main(void) { return FOO; }\n");
    EXPECT_TRUE(needsGcc(path, withPreprocessorArgs({ "-D", "FOO=1" })));
    std::filesystem::remove(path);
}

TEST(PreprocessSkip, includePathArgForcesGccWithoutHashInSource) {
    auto path = writeTemp("include_path_arg.c", "int main(void) { return 0; }\n");
    EXPECT_TRUE(needsGcc(path, withPreprocessorArgs({ "-I", "inc" })));
    std::filesystem::remove(path);
}

TEST(PreprocessSkip, includeFileArgForcesGccWithoutHashInSource) {
    auto path = writeTemp("include_file_arg.c", "int main(void) { return 0; }\n");
    EXPECT_TRUE(needsGcc(path, withPreprocessorArgs({ "-include", "hdr.h" })));
    std::filesystem::remove(path);
}

TEST(PreprocessSkip, stdFlagAloneDoesNotForceGcc) {
    auto path = writeTemp("std_only.c", "int main(void) { return 0; }\n");
    Configuration configuration;
    configuration.setPreprocessorStdFlag("c11");
    EXPECT_FALSE(needsGcc(path, configuration));
    std::filesystem::remove(path);
}

TEST(PreprocessSkip, missingFileNeedsGcc) {
    EXPECT_TRUE(Compiler::sourceFileNeedsGccPreprocessor("/no/such/trans_preprocess_skip.c", emptyConfig()));
}

TEST(PreprocessSkip, compileWithoutHashLeavesNoDotI) {
    auto path = writeTemp("compile_no_hash.c", "int main(void) { return 0; }\n");
    auto iPath = path.string() + ".i";
    std::filesystem::remove(iPath);

    Compiler compiler { productConfig() };
    ASSERT_NO_THROW(compiler.compile(path.string()));
    EXPECT_FALSE(std::filesystem::exists(iPath));

    std::filesystem::remove(path);
    std::filesystem::remove(path.string() + ".S");
    std::filesystem::remove(path.string() + ".o");
}

TEST(PreprocessSkip, compileWithIncludeStillWorks) {
    auto path = writeTemp("compile_with_include.c",
            "#include <stddef.h>\nint main(void) { return sizeof(size_t) > 0 ? 0 : 1; }\n");
    Compiler compiler { productConfig() };
    ASSERT_NO_THROW(compiler.compile(path.string()));

    std::filesystem::remove(path);
    std::filesystem::remove(path.string() + ".i");
    std::filesystem::remove(path.string() + ".S");
    std::filesystem::remove(path.string() + ".o");
}

} // namespace
