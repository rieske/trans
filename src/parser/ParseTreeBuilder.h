#ifndef PARSETREEBUILDER_H_
#define PARSETREEBUILDER_H_

#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "../parser/GrammarSymbol.h"
#include "../parser/ParseTreeNode.h"

class SyntaxTree;
class Token;

class ParseTreeBuilder {
public:
	ParseTreeBuilder();
	virtual ~ParseTreeBuilder();

	virtual std::unique_ptr<SyntaxTree> build();

	virtual void makeTerminalNode(const Token& token);
	virtual void makeNonterminalNode(std::string definingSymbol, Production production);

protected:
	std::vector<ParseTreeNode*> getChildrenForReduction(int childrenCount);

	SyntaxTree* syntaxTree;
	std::stack<ParseTreeNode *> syntaxStack;

	std::string sourceFileName;
};

#endif /* PARSETREEBUILDER_H_ */
