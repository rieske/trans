#include "ParseTreeBuilder.h"

#include <algorithm>

#include "ParseTree.h"
#include "TerminalNode.h"

namespace parser {

ParseTreeBuilder::ParseTreeBuilder(std::string sourceFileName, const Grammar* grammar):
    sourceFileName {sourceFileName},
    grammar {grammar}
{}

ParseTreeBuilder::~ParseTreeBuilder() = default;

void ParseTreeBuilder::makeNonterminalNode(int definingSymbol, parser::Production production) {
    std::vector<std::unique_ptr<ParseTreeNode>> children = getChildrenForReduction(production.size());
    syntaxStack.push(std::make_unique<ParseTreeNode>(grammar->getSymbolById(definingSymbol), std::move(children)));
}

void ParseTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context&) {
    syntaxStack.push(std::make_unique<TerminalNode>(type, value));
}

std::unique_ptr<SyntaxTree> ParseTreeBuilder::build() {
    return std::make_unique<ParseTree>(std::move(syntaxStack.top()));
}

std::vector<std::unique_ptr<ParseTreeNode>> ParseTreeBuilder::getChildrenForReduction(int childrenCount) {
    std::vector<std::unique_ptr<ParseTreeNode>> children;
    for (int i = childrenCount; i > 0; --i) {
        children.push_back(std::move(syntaxStack.top()));
        syntaxStack.pop();
    }
    std::reverse(children.begin(), children.end());
    return children;
}

} // namespace parser

