#include "ParseTreeBuilder.h"

#include <algorithm>
#include <fstream>

#include "ParseTree.h"
#include "TerminalNode.h"

using std::string;
using std::vector;

namespace parser {

ParseTreeBuilder::ParseTreeBuilder(std::string sourceFileName):
    sourceFileName {sourceFileName}
{}

ParseTreeBuilder::~ParseTreeBuilder() {
}

void ParseTreeBuilder::makeNonterminalNode(string definingSymbol, parser::Production production) {
    vector<std::unique_ptr<ParseTreeNode>> children = getChildrenForReduction(production.size());
    syntaxStack.push(std::make_unique<ParseTreeNode>(definingSymbol, std::move(children)));
}

void ParseTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context&) {
    syntaxStack.push(std::make_unique<TerminalNode>(type, value));
}

std::unique_ptr<SyntaxTree> ParseTreeBuilder::build() {
    auto parseTree = std::make_unique<ParseTree>(std::move(syntaxStack.top()));

    std::ofstream xmlStream { sourceFileName + ".syntax.xml" };
    parseTree->outputXml(xmlStream);
    std::ofstream sourceCodeStream { sourceFileName + ".parse.src" };
    parseTree->outputSource(sourceCodeStream);

    return std::move(parseTree);
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
