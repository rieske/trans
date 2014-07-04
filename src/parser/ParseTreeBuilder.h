#ifndef PARSETREEBUILDER_H_
#define PARSETREEBUILDER_H_

#include <stack>
#include <string>
#include <vector>

#include "GrammarSymbol.h"
#include "ParseTreeNode.h"

class SyntaxTree;
class Token;

class ParseTreeBuilder {
public:
	ParseTreeBuilder();
	virtual ~ParseTreeBuilder();

	virtual SyntaxTree build();

	virtual void makeTerminalNode(const Token& token);
	virtual void makeNonterminalNode(std::string definingSymbol, Production production);

protected:
	std::vector<ParseTreeNode*> getChildrenForReduction(int childrenCount);

	std::stack<ParseTreeNode *> syntaxStack;

	std::string sourceFileName;
};

#endif /* PARSETREEBUILDER_H_ */
