#include "ParseTreeBuilder.h"

#include <memory>

#include "../parser/nonterminal_node.h"
#include "../parser/terminal_node.h"
#include "../scanner/Token.h"
#include "SyntaxTree.h"

using std::string;
using std::vector;
using std::unique_ptr;

ParseTreeBuilder::ParseTreeBuilder() :
		syntaxTree { new SyntaxTree() } {
}

ParseTreeBuilder::~ParseTreeBuilder() {
}

void ParseTreeBuilder::makeNonTerminalNode(string left, int childrenCount, string reduction) {
	vector<Node *> children = getChildrenForReduction(childrenCount);

	Node *n_node = new NonterminalNode(left, children, reduction);
	if (true == n_node->getErrorFlag()) {
		syntaxTree->setErrorFlag();
	}
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

vector<Node*> ParseTreeBuilder::getChildrenForReduction(int childrenCount) {
	vector<Node*> children;
	for (int i = childrenCount; i > 0; i--) {
		children.push_back(syntaxStack.top());
		syntaxStack.pop();
	}
	return children;
}
