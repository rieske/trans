#ifndef ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_
#define ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_

#include <memory>
#include <stack>

#include "TerminalSymbol.h"

class SymbolTable;

namespace semantic_analyzer {

class Expression;
class AssignmentExpressionList;
class TerminalSymbol;
class Pointer;
class AbstractSyntaxTreeNode;
class LoopHeader;

class AbstractSyntaxTreeBuilderContext {
public:
	AbstractSyntaxTreeBuilderContext(SymbolTable* currentScope);
	virtual ~AbstractSyntaxTreeBuilderContext();

	SymbolTable* scope();
	void innerScope();
	void outerScope();
	int line() const;
	void setLine(int line);

	void pushTerminal(TerminalSymbol terminal);
	TerminalSymbol popTerminal();

	void pushExpression(std::unique_ptr<Expression> expression);
	std::unique_ptr<Expression> popExpression();

	void pushAssignmentExpressionList(std::unique_ptr<AssignmentExpressionList> assignmentExpressions);
	std::unique_ptr<AssignmentExpressionList> popAssignmentExpressionList();

	void pushPointer(std::unique_ptr<Pointer> pointer);
	std::unique_ptr<Pointer> popPointer();

	void pushLoopHeader(std::unique_ptr<LoopHeader> loopHeader);
	std::unique_ptr<LoopHeader> popLoopHeader();

	void pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> statement);
	std::unique_ptr<AbstractSyntaxTreeNode> popStatement();

private:
	// FIXME:
	int currentLine { 0 };
	SymbolTable* currentScope;

	std::stack<TerminalSymbol> terminalSymbols;

	std::stack<std::unique_ptr<Expression>> expressionStack;
	std::stack<std::unique_ptr<AssignmentExpressionList>> assignmentExpressioListStack;
	std::stack<std::unique_ptr<Pointer>> pointerStack;
	std::stack<std::unique_ptr<LoopHeader>> loopHeaderStack;
	std::stack<std::unique_ptr<AbstractSyntaxTreeNode>> statementStack;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_ */
