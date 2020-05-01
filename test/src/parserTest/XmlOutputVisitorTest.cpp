#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <sstream>

#include "../parser/XmlOutputVisitor.h"
#include "../parser/TerminalNode.h"

using namespace parser;

using testing::Eq;

namespace {

TEST(XmlOutputVisitor, usesNodeTypeAsXmlNodeName) {
	ParseTreeNode node { "node", { } };

	std::ostringstream stream;
	XmlOutputVisitor visitor { &stream };

	node.accept(visitor);

	EXPECT_THAT(stream.str(), Eq("<node>\n"
			"</node>\n"));
}

TEST(XmlOutputVisitor, stripsMoreAndLessCharactersFromNodeName) {
	ParseTreeNode node { "<node>", { } };

	std::ostringstream stream;
	XmlOutputVisitor visitor { &stream };

	node.accept(visitor);

	EXPECT_THAT(stream.str(), Eq("<node>\n"
			"</node>\n"));
}

TEST(XmlOutputVisitor, convertsTerminalNodeToAnElementWithAttributes) {
	TerminalNode node { "type", "value" };

	std::ostringstream stream;
	XmlOutputVisitor visitor { &stream };

	node.accept(visitor);

	EXPECT_THAT(stream.str(), Eq("<terminal type='type' value='value'/>\n"));
}

TEST(XmlOutputVisitor, convertsNodeTreeToXml) {
	std::vector<std::unique_ptr<ParseTreeNode>> children;
	children.push_back(std::unique_ptr<ParseTreeNode> { new ParseTreeNode("firstChild", { }) });
	children.push_back(std::unique_ptr<ParseTreeNode> { new ParseTreeNode("secondChild", { }) });
	children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("type", "value") });
	ParseTreeNode parent { "parent", std::move(children) };

	std::ostringstream stream;
	XmlOutputVisitor visitor { &stream };

	parent.accept(visitor);

	EXPECT_THAT(stream.str(), Eq("<parent>\n"
			"  <firstChild>\n"
			"  </firstChild>\n"
			"  <secondChild>\n"
			"  </secondChild>\n"
			"  <terminal type='type' value='value'/>\n"
			"</parent>\n"));
}

}
