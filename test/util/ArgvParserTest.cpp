#include "gtest/gtest.h"
#include "util/ArgvParser.h"

TEST(ArgvParserTest, ShouldCreateArgvParser) {
	ArgvParser *argvParser = new ArgvParser();
    EXPECT_EQ(1, 1);
}
