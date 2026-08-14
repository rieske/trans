#include "gtest/gtest.h"

#include "util/SourcePath.h"

namespace {

TEST(SourcePath, classifiesSourceAndCompileArtifacts) {
    EXPECT_EQ(util::classifyInput("foo.c"), util::InputKind::Source);
    EXPECT_EQ(util::classifyInput("foo.i"), util::InputKind::Preprocessed);
    EXPECT_EQ(util::classifyInput("foo.s"), util::InputKind::Assembly);
    EXPECT_EQ(util::classifyInput("foo.S"), util::InputKind::Assembly);
    EXPECT_EQ(util::classifyInput("foo.o"), util::InputKind::Object);
}

TEST(SourcePath, classifiesArchiveAndSharedObjectSuffixesAsObject) {
    EXPECT_EQ(util::classifyInput("libgit.a"), util::InputKind::Object);
    EXPECT_EQ(util::classifyInput("/tmp/libfoo.a"), util::InputKind::Object);
    EXPECT_EQ(util::classifyInput("libfoo.so"), util::InputKind::Object);
}

TEST(SourcePath, versionedSharedObjectIsNotASuffixMatch) {
    EXPECT_EQ(util::classifyInput("libfoo.so.1"), util::InputKind::Source);
    EXPECT_EQ(util::classifyInput("/usr/lib/libfoo.so.1.2.3"), util::InputKind::Source);
}

TEST(SourcePath, soInsideASourceNameIsStillSource) {
    EXPECT_EQ(util::classifyInput("file.so.c"), util::InputKind::Source);
    EXPECT_EQ(util::classifyInput("also.so.h"), util::InputKind::Source);
}

} // namespace
