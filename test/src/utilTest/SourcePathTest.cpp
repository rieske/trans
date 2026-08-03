#include "util/SourcePath.h"

#include "gtest/gtest.h"

using util::InputKind;
using util::classifyInput;
using util::isLinkInput;

TEST(SourcePath, classifiesSourceAndCompileArtifacts) {
    EXPECT_EQ(classifyInput("foo.c"), InputKind::Source);
    EXPECT_EQ(classifyInput("foo.i"), InputKind::Preprocessed);
    EXPECT_EQ(classifyInput("foo.s"), InputKind::Assembly);
    EXPECT_EQ(classifyInput("foo.S"), InputKind::Assembly);
    EXPECT_EQ(classifyInput("foo.o"), InputKind::LinkInput);
}

TEST(SourcePath, classifiesLinkInputs) {
    EXPECT_TRUE(isLinkInput("a.o"));
    EXPECT_TRUE(isLinkInput("libgit.a"));
    EXPECT_TRUE(isLinkInput("target/release/libgitcore.a"));
    EXPECT_TRUE(isLinkInput("libfoo.so"));
    EXPECT_TRUE(isLinkInput("libfoo.so.1"));
    EXPECT_TRUE(isLinkInput("/usr/lib/libfoo.so.1.2.3"));
    EXPECT_FALSE(isLinkInput("foo.so.c"));
    EXPECT_FALSE(isLinkInput("also.so.h"));
    EXPECT_FALSE(isLinkInput("a.c"));
    EXPECT_FALSE(isLinkInput("a.s"));
    EXPECT_FALSE(isLinkInput("a.i"));

    EXPECT_EQ(classifyInput("x.o"), InputKind::LinkInput);
    EXPECT_EQ(classifyInput("lib.a"), InputKind::LinkInput);
    EXPECT_EQ(classifyInput("lib.so.2"), InputKind::LinkInput);
    EXPECT_EQ(classifyInput("/tmp/libfoo.a"), InputKind::LinkInput);
    EXPECT_EQ(classifyInput("libfoo.so"), InputKind::LinkInput);
    EXPECT_EQ(classifyInput("x.s"), InputKind::Assembly);
    EXPECT_EQ(classifyInput("x.S"), InputKind::Assembly);
    EXPECT_EQ(classifyInput("x.i"), InputKind::Preprocessed);
    EXPECT_EQ(classifyInput("x.c"), InputKind::Source);
}

TEST(SourcePath, soInsideASourceNameIsStillSource) {
    EXPECT_EQ(classifyInput("file.so.c"), InputKind::Source);
    EXPECT_EQ(classifyInput("also.so.h"), InputKind::Source);
}
