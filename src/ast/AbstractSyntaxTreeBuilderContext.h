#ifndef ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_
#define ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_

#include <memory>
#include <stack>
#include <vector>

#include "TerminalSymbol.h"
#include "TypeSpecifier.h"

class SymbolTable;

namespace ast {

class Expression;
class AssignmentExpressionList;
class TerminalSymbol;
class Pointer;
class AbstractSyntaxTreeNode;
class LoopHeader;
class Declaration;
class DeclarationList;
class FunctionDeclaration;
class ParameterDeclaration;
class ParameterList;
class VariableDeclaration;
class ListCarrier;

class AbstractSyntaxTreeBuilderContext {
public:
	AbstractSyntaxTreeBuilderContext();
	virtual ~AbstractSyntaxTreeBuilderContext();

	int line() const;
	void setLine(int line);

	void pushTerminal(TerminalSymbol terminal);
	TerminalSymbol popTerminal();

	void pushTypeSpecifier(TypeSpecifier typeSpecifier);
	TypeSpecifier popTypeSpecifier();

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

	void pushDeclaration(std::unique_ptr<Declaration> declaration);
	std::unique_ptr<Declaration> popDeclaration();

	void pushDeclarationList(std::unique_ptr<DeclarationList> declarationList);
	std::unique_ptr<DeclarationList> popDeclarationList();

	void pushFunctionDeclaration(std::unique_ptr<FunctionDeclaration> declaration);
	std::unique_ptr<FunctionDeclaration> popFunctionDeclaration();

	void pushParameter(std::unique_ptr<ParameterDeclaration> parameter);
	std::unique_ptr<ParameterDeclaration> popParameter();

	void pushParameterList(std::unique_ptr<ParameterList> parameters);
	std::unique_ptr<ParameterList> popParameterList();

	void pushListCarrier(std::unique_ptr<ListCarrier> carrier);
	std::unique_ptr<ListCarrier> popListCarrier();

private:
	// FIXME:
	int currentLine { 0 };

	std::stack<TerminalSymbol> terminalSymbols;

	std::stack<TypeSpecifier> typeSpecifiers;

	std::stack<std::unique_ptr<Expression>> expressionStack;
	std::stack<std::unique_ptr<AssignmentExpressionList>> assignmentExpressioListStack;
	std::stack<std::unique_ptr<Pointer>> pointerStack;
	std::stack<std::unique_ptr<LoopHeader>> loopHeaderStack;
	std::stack<std::unique_ptr<AbstractSyntaxTreeNode>> statementStack;
	std::stack<std::unique_ptr<Declaration>> declarationStack;
	std::stack<std::unique_ptr<DeclarationList>> declarationListStack;
	std::stack<std::unique_ptr<FunctionDeclaration>> functionDeclarationStack;
	std::stack<std::unique_ptr<ParameterDeclaration>> parameterStack;
	std::stack<std::unique_ptr<ParameterList>> parameterListStack;
	std::stack<std::unique_ptr<ListCarrier>> listCarrierStack;
};

} /* namespace ast */

#endif /* ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_ */
