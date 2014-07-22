#include "AbstractSyntaxTreeBuilderContext.h"
#include "Expression.h"

#include <algorithm>

namespace semantic_analyzer {

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext() {
}

AbstractSyntaxTreeBuilderContext::~AbstractSyntaxTreeBuilderContext() {
}

void AbstractSyntaxTreeBuilderContext::pushTerminal(TerminalSymbol terminal) {
	terminalSymbols.push(terminal);
}

TerminalSymbol AbstractSyntaxTreeBuilderContext::popTerminal() {
	TerminalSymbol top = terminalSymbols.top();
	terminalSymbols.pop();
	return top;
}

void AbstractSyntaxTreeBuilderContext::pushExpression(std::unique_ptr<Expression> expression) {
	expressionStack.push(std::move(expression));
}

std::unique_ptr<Expression> AbstractSyntaxTreeBuilderContext::popExpression() {
	std::unique_ptr<Expression> expression = std::move(expressionStack.top());
	expressionStack.pop();
	return expression;
}

} /* namespace semantic_analyzer */
