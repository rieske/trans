#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "DriverHarness.h"
#include "ResourceHelpers.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

using namespace testing;

TEST(Driver, writesUserHeaderDependencyFile) {
    auto dir = std::filesystem::temp_directory_path() / "trans_header_dep_tests";
    std::filesystem::create_directories(dir);
    ScopedTempFile header { (dir / "dep_hdr.h").string() };
    ScopedTempFile source { (dir / "dep_src.c").string() };
    ScopedTempFile object { (dir / "dep_src.o").string() };
    ScopedTempFile dep { (dir / "dep_src.d").string() };
    {
        std::ofstream out { header.path() };
        out << "enum { DEP_HDR = 1 };\n";
    }
    {
        std::ofstream out { source.path() };
        out << "#include \"dep_hdr.h\"\nint main(void) { return DEP_HDR - 1; }\n";
    }
    ArgvBuffer args {
            { source.path() },
            { "-c", "-o", object.path(), "-MMD", "-MP", "-MF", dep.path(), "-MQ", object.path() }
    };
    std::string errors;
    std::string output;
    EXPECT_EQ(runDriver(args, &errors, &output), 0) << errors;
    EXPECT_THAT(output, Eq(""));
    ASSERT_TRUE(std::filesystem::exists(dep.path()));
    std::ifstream in { dep.path() };
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_THAT(contents, HasSubstr(object.path() + ":"));
    EXPECT_THAT(contents, HasSubstr(source.path()));
    EXPECT_THAT(contents, HasSubstr(header.path()));
    EXPECT_THAT(contents, HasSubstr(header.path() + ":"));
}

TEST(Driver, gitHeaderDepProbeRejectsEmptyTranslationUnit) {
    ArgvBuffer args {
            { "/dev/null" },
            { "-g", "-O2", "-Wall", "-Wno-pedantic", "-c",
                    "-MF", "/dev/null", "-MQ", "/dev/null", "-MMD", "-MP",
                    "-x", "c", "-o", "/dev/null" }
    };
    std::string errors;
    std::string output;
    EXPECT_NE(runDriver(args, &errors, &output), 0);
    EXPECT_THAT(errors, HasSubstr("unexpected token"));
}
