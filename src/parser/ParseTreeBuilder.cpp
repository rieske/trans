#include "ParseTreeBuilder.h"

#include <algorithm>

#include "../scanner/Token.h"
#include "NonterminalNode.h"
#include "SyntaxTree.h"
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

void ParseTreeBuilder::makeTerminalNode(const Token& token) {
	TerminalNode *t_node = new TerminalNode(token.id, token.lexeme);
	syntaxStack.push(t_node);
}

SyntaxTree ParseTreeBuilder::build() {
	return {syntaxStack.top(), nullptr};
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
