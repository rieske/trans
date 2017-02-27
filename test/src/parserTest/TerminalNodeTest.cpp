#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

#include "parser/TerminalNode.h"

using namespace parser;

using testing::Eq;
using testing::SizeIs;

namespace {

TEST(TerminalNode, containsTypeAndValue) {
	TerminalNode terminalNode("type", "value");

	EXPECT_THAT(terminalNode.getType(), Eq("type"));
	EXPECT_THAT(terminalNode.getValue(), Eq("value"));
}

TEST(TerminalNode, doesNotHaveChildren) {
	TerminalNode terminalNode("type", "value");

	EXPECT_THAT(terminalNode.getChildren(), SizeIs(0));
}

}
