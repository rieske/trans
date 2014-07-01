#include "ParseTreeBuilder.h"

#include "../scanner/Token.h"
#include "nonterminal_node.h"
#include "SyntaxTree.h"
#include "terminal_node.h"

using std::string;
using std::vector;
using std::unique_ptr;

ParseTreeBuilder::ParseTreeBuilder() :
		syntaxTree { new SyntaxTree() } {
}

ParseTreeBuilder::~ParseTreeBuilder() {
}

void ParseTreeBuilder::makeNonterminalNode(string definingSymbol, Production production) {
	vector<ParseTreeNode *> children = getChildrenForReduction(production.size());

	ParseTreeNode *n_node = new NonterminalNode(definingSymbol, children, production);
	syntaxStack.push(n_node);
}

void ParseTreeBuilder::makeTerminalNode(const Token& token) {
	TerminalNode *t_node = new TerminalNode(token.id, token.lexeme);
	syntaxStack.push(t_node);
}

unique_ptr<SyntaxTree> ParseTreeBuilder::build() {
	syntaxTree->setTree(syntaxStack.top());
	return std::unique_ptr<SyntaxTree> { syntaxTree };
}

void ParseTreeBuilder::withSourceFileName(std::string fileName) {
	this->sourceFileName = fileName;

	syntaxTree->setFileName(sourceFileName.c_str());
}

vector<ParseTreeNode*> ParseTreeBuilder::getChildrenForReduction(int childrenCount) {
	vector<ParseTreeNode*> children;
	for (int i = childrenCount; i > 0; i--) {
		children.push_back(syntaxStack.top());
		syntaxStack.pop();
	}
	return children;
}
