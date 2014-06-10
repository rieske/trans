#ifndef PARSETREEBUILDER_H_
#define PARSETREEBUILDER_H_

#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "../parser/node.h"

class SyntaxTree;
class Token;

class ParseTreeBuilder {
public:
	ParseTreeBuilder();
	virtual ~ParseTreeBuilder();

	virtual std::unique_ptr<SyntaxTree> build();

	void withSourceFileName(std::string fileName);

	virtual void makeTerminalNode(string terminal, Token token);
	virtual void makeNonTerminalNode(std::string left, int childrenCount, std::string reduction);

protected:
	std::vector<Node*> getChildrenForReduction(int childrenCount);

	SyntaxTree* syntaxTree;
	std::stack<Node *> syntaxStack;

	std::string sourceFileName;
};

#endif /* PARSETREEBUILDER_H_ */
