#ifndef ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_
#define ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_

#include <memory>
#include <stack>

#include "TerminalSymbol.h"

namespace semantic_analyzer {

class Expression;

class AbstractSyntaxTreeBuilderContext {
public:
	AbstractSyntaxTreeBuilderContext();
	virtual ~AbstractSyntaxTreeBuilderContext();

	void pushTerminal(TerminalSymbol terminal);
	TerminalSymbol popTerminal();

	void pushExpression(std::unique_ptr<Expression> expression);
	std::unique_ptr<Expression> popExpression();

private:
	std::stack<TerminalSymbol> terminalSymbols;

	std::stack<std::unique_ptr<Expression>> expressionStack;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_ */
