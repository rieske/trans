#include "../parser/ParseTreeToSourceConverter.h"

#include "../parser/TerminalNode.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

using namespace parser;

using testing::Eq;

namespace {

TEST(ParseTreeToSourceConverterTest, outputsTerminalSymbolsInOrderOfAppearance) {
    std::vector<std::unique_ptr<ParseTreeNode>> children;
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("type1", "value1") });
    std::vector<std::unique_ptr<ParseTreeNode>> secondLevelChildren;
    secondLevelChildren.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("type2", "value2") });
    secondLevelChildren.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("type3", "value3") });
    std::unique_ptr<ParseTreeNode> nonterminalChild { new ParseTreeNode("nonterminalChild",
            std::move(secondLevelChildren)) };
    children.push_back(std::move(nonterminalChild));
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("type4", "value4") });
    ParseTreeNode parent { "parent", std::move(children) };

    std::ostringstream stream;
    ParseTreeToSourceConverter converter { &stream };

    parent.accept(converter);

    EXPECT_THAT(stream.str(), Eq("value1 value2 value3 value4"));
}

TEST(ParseTreeToSourceConverterTest, delimitsSourceFileElements) {
    std::vector<std::unique_ptr<ParseTreeNode>> children;
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("int", "int") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("id", "functionName") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("(", "(") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("char", "char") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("*", "*") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("id", "paramName") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode(")", ")") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("{", "{") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("id", "functionCall") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("(", "(") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("id", "arg") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode(")", ")") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode(";", ";") });
    children.push_back(std::unique_ptr<ParseTreeNode> { new TerminalNode("}", "}") });
    ParseTreeNode parent { "parent", std::move(children) };

    std::ostringstream stream;
    ParseTreeToSourceConverter converter { &stream };

    parent.accept(converter);

    EXPECT_THAT(stream.str(), Eq("int functionName ( char * paramName ) {\n"
            "    functionCall ( arg ) ;\n"
            "}"));
}

}
