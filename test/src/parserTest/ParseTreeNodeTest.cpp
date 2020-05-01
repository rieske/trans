#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

#include "../parser/ParseTreeNode.h"

using namespace parser;

using testing::Eq;
using testing::SizeIs;

namespace {

TEST(ParseTreeNode, namesAType) {
	ParseTreeNode node { "type", { } };

	EXPECT_THAT(node.getType(), Eq("type"));
}

TEST(ParseTreeNode, ownsItsChildren) {
	std::vector<std::unique_ptr<ParseTreeNode>> children;
	children.push_back(std::unique_ptr<ParseTreeNode> { new ParseTreeNode("firstChild", { }) });
	children.push_back(std::unique_ptr<ParseTreeNode> { new ParseTreeNode("secondChild", { }) });
	ParseTreeNode node { "type", std::move(children) };

	EXPECT_THAT(node.getType(), Eq("type"));
	EXPECT_THAT(node.getChildren(), SizeIs(2));
	EXPECT_THAT(node.getChildren().at(0)->getType(), Eq("firstChild"));
	EXPECT_THAT(node.getChildren().at(1)->getType(), Eq("secondChild"));
}

}
