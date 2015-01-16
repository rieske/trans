#ifndef ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_
#define ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_

#include <memory>
#include <stack>

#include "FunctionDeclarator.h"
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
class Declarator;
class DeclarationList;
class FormalArgument;
class VariableDeclaration;
class ListCarrier;

class AbstractSyntaxTreeBuilderContext {
public:
    AbstractSyntaxTreeBuilderContext();
    virtual ~AbstractSyntaxTreeBuilderContext();

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

    void pushDeclarator(std::unique_ptr<Declarator> declarator);
    std::unique_ptr<Declarator> popDeclarator();

    void pushDeclarationList(std::unique_ptr<DeclarationList> declarationList);
    std::unique_ptr<DeclarationList> popDeclarationList();

    void pushFunctionDeclaration(std::unique_ptr<FunctionDeclarator> declaration);
    std::unique_ptr<FunctionDeclarator> popFunctionDeclaration();

    void pushFormalArgument(std::unique_ptr<FormalArgument> formalArgument);
    std::unique_ptr<FormalArgument> popFormalArgument();

    void pushFormalArguments(std::unique_ptr<FormalArguments> formalArguments);
    std::unique_ptr<FormalArguments> popFormalArguments();

    void pushListCarrier(std::unique_ptr<ListCarrier> carrier);
    std::unique_ptr<ListCarrier> popListCarrier();

private:
    std::stack<TerminalSymbol> terminalSymbols;

    std::stack<TypeSpecifier> typeSpecifiers;

    std::stack<std::unique_ptr<Expression>> expressionStack;
    std::stack<std::unique_ptr<AssignmentExpressionList>> assignmentExpressioListStack;
    std::stack<std::unique_ptr<Pointer>> pointerStack;
    std::stack<std::unique_ptr<LoopHeader>> loopHeaderStack;
    std::stack<std::unique_ptr<AbstractSyntaxTreeNode>> statementStack;
    std::stack<std::unique_ptr<Declarator>> declarators;
    std::stack<std::unique_ptr<DeclarationList>> declarationListStack;
    std::stack<std::unique_ptr<FunctionDeclarator>> functionDeclarationStack;
    std::stack<std::unique_ptr<FormalArgument>> formalArguments;
    std::stack<std::unique_ptr<FormalArguments>> formalArgumentLists;
    std::stack<std::unique_ptr<ListCarrier>> listCarrierStack;
};

}
/* namespace ast */

#endif /* ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_ */
