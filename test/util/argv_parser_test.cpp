#include "gtest/gtest.h"
#include "util/argv_parser.h"

TEST(ArgvParserTest, ShouldCreateArgvParser) {
	Argv_parser *argv_parser = new Argv_parser();
    EXPECT_EQ(1, 1);
}
