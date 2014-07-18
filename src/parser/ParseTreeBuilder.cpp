#include "ParseTreeBuilder.h"

#include <algorithm>

#include "../scanner/Token.h"
#include "NonterminalNode.h"
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
	vector<ParseTreeNode *> children = getChildrenForReduction(production.size());

	ParseTreeNode *n_node = new NonterminalNode(definingSymbol, children);
	syntaxStack.push(n_node);
}

void ParseTreeBuilder::makeTerminalNode(std::string type, std::string value, size_t line) {
	TerminalNode *t_node = new TerminalNode(type, value);
	syntaxStack.push(t_node);
}

std::unique_ptr<SyntaxTree> ParseTreeBuilder::build() {
	return std::unique_ptr<SyntaxTree>(new ParseTree(std::unique_ptr<ParseTreeNode> { syntaxStack.top() }));
}

vector<ParseTreeNode*> ParseTreeBuilder::getChildrenForReduction(int childrenCount) {
	vector<ParseTreeNode*> children;
	for (int i = childrenCount; i > 0; --i) {
		children.push_back(syntaxStack.top());
		syntaxStack.pop();
	}
	std::reverse(children.begin(), children.end());
	return children;
}

}
