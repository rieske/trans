#ifndef SYNTAXTREEBUILDER_H_
#define SYNTAXTREEBUILDER_H_

#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "../parser/node.h"

class SymbolTable;
class Token;

class SyntaxTree;

class SyntaxTreeBuilder {
public:
	SyntaxTreeBuilder();
	virtual ~SyntaxTreeBuilder();

	virtual std::unique_ptr<SyntaxTree> build();
	virtual void withSourceFileName(std::string fileName);

	virtual void makeTerminalNode(std::string terminal, Token token) = 0;
	virtual void makeNonTerminalNode(std::string left, int childrenCount, std::string reduction) = 0;

protected:
	std::vector<Node*> getChildrenForReduction(int childrenCount);

	int currentLine { 0 };
	SyntaxTree* syntaxTree;
	SymbolTable* currentScope;
	std::stack<Node *> syntaxStack;

	std::string sourceFileName;
};

#endif /* SYNTAXTREEBUILDER_H_ */
