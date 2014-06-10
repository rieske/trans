#include "ParseTreeBuilder.h"

#include <stack>

#include "../parser/nonterminal_node.h"
#include "../parser/terminal_node.h"
#include "../scanner/Token.h"
#include "SyntaxTree.h"

using std::string;
using std::vector;

ParseTreeBuilder::ParseTreeBuilder() {
	// TODO Auto-generated constructor stub

}

ParseTreeBuilder::~ParseTreeBuilder() {
	// TODO Auto-generated destructor stub
}

void ParseTreeBuilder::makeNonTerminalNode(string left, int childrenCount, string reduction) {
	vector<Node *> children = getChildrenForReduction(childrenCount);

	Node *n_node = new NonterminalNode(left, children, reduction);
	if (true == n_node->getErrorFlag()) {
		syntaxTree->setErrorFlag();
	}
	syntaxStack.push(n_node);
}

void ParseTreeBuilder::makeTerminalNode(string terminal, Token token) {
	TerminalNode *t_node = new TerminalNode(terminal, token.getLexeme());
	currentLine = token.line;
	syntaxStack.push(t_node);
}
