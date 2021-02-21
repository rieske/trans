#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

#include "parser/ParseTree.h"
#include "parser/ParseTreeNode.h"
#include "parser/TerminalNode.h"
#include "parser/XmlOutputVisitor.h"

using namespace parser;

using testing::Eq;

namespace {

TEST(ParseTree, outputsTreeAsXmlStream) {
    std::vector<std::unique_ptr<ParseTreeNode>> children;
    children.push_back(std::unique_ptr<ParseTreeNode> { new ParseTreeNode("firstChild", {}) });
    children.push_back(std::unique_ptr<ParseTreeNode> { new ParseTreeNode("secondChild", { }) });
    children.push_back(std::make_unique<TerminalNode>("type", "value"));
    std::unique_ptr<ParseTreeNode> parent { new ParseTreeNode("parent", std::move(children)) };

    std::ostringstream stream;
    ParseTree tree { std::move(parent) };
    XmlOutputVisitor toXml { &stream };
    tree.accept(toXml);

    EXPECT_THAT(stream.str(), Eq("<parent>\n"
            "  <firstChild>\n"
            "  </firstChild>\n"
            "  <secondChild>\n"
            "  </secondChild>\n"
            "  <terminal type='type' value='value'/>\n"
            "</parent>\n"));
}

}
