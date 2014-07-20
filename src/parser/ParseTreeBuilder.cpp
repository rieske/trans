#include "ParseTreeBuilder.h"

#include <algorithm>

#include "ParseTree.h"
#include "TerminalNode.h"

using std::string;
using std::vector;

namespace parser {

ParseTreeBuilder::ParseTreeBuilder() {
}

ParseTreeBuilder::~ParseTreeBuilder() {
}

void ParseTreeBuilder::makeNonterminalNode(string definingSymbol, parser::Production production) {
    vector<std::unique_ptr<ParseTreeNode>> children = getChildrenForReduction(production.size());
    syntaxStack.push(std::unique_ptr<ParseTreeNode> { new ParseTreeNode(definingSymbol, std::move(children)) });
}

void ParseTreeBuilder::makeTerminalNode(std::string type, std::string value, size_t) {
    syntaxStack.push(std::unique_ptr<ParseTreeNode> { new TerminalNode(type, value) });
}

std::unique_ptr<SyntaxTree> ParseTreeBuilder::build() {
    return std::unique_ptr<SyntaxTree>(new ParseTree(std::unique_ptr<ParseTreeNode> { std::move(syntaxStack.top()) }));
}

vector<std::unique_ptr<ParseTreeNode>> ParseTreeBuilder::getChildrenForReduction(int childrenCount) {
    vector<std::unique_ptr<ParseTreeNode>> children;
    for (int i = childrenCount; i > 0; --i) {
        children.push_back(std::move(syntaxStack.top()));
        syntaxStack.pop();
    }
    std::reverse(children.begin(), children.end());
    return children;
}

}
