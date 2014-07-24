#include "AbstractSyntaxTreeBuilderContext.h"

#include <algorithm>

#include "../code_generator/symbol_table.h"
#include "TerminalSymbol.h"
#include "Expression.h"
#include "AssignmentExpressionList.h"
#include "Pointer.h"

namespace semantic_analyzer {

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext(SymbolTable* currentScope) :
		currentScope { currentScope } {
}

AbstractSyntaxTreeBuilderContext::~AbstractSyntaxTreeBuilderContext() {
}

SymbolTable* AbstractSyntaxTreeBuilderContext::scope() {
	return currentScope;
}

void AbstractSyntaxTreeBuilderContext::innerScope() {
	currentScope = currentScope->newScope();
}

void AbstractSyntaxTreeBuilderContext::outerScope() {
	currentScope = currentScope->getOuterScope();
}

int AbstractSyntaxTreeBuilderContext::line() const {
	return currentLine;
}

void AbstractSyntaxTreeBuilderContext::setLine(int line) {
	this->currentLine = line;
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

void AbstractSyntaxTreeBuilderContext::pushAssignmentExpressionList(std::unique_ptr<AssignmentExpressionList> assignmentExpressions) {
	assignmentExpressioListStack.push(std::move(assignmentExpressions));
}

std::unique_ptr<AssignmentExpressionList> AbstractSyntaxTreeBuilderContext::popAssignmentExpressionList() {
	std::unique_ptr<AssignmentExpressionList> assignmentExpressions = std::move(assignmentExpressioListStack.top());
	assignmentExpressioListStack.pop();
	return assignmentExpressions;
}

void AbstractSyntaxTreeBuilderContext::pushPointer(std::unique_ptr<Pointer> pointer) {
	pointerStack.push(std::move(pointer));
}

std::unique_ptr<Pointer> AbstractSyntaxTreeBuilderContext::popPointer() {
	std::unique_ptr<Pointer> pointer = std::move(pointerStack.top());
	pointerStack.pop();
	return pointer;
}

} /* namespace semantic_analyzer */
