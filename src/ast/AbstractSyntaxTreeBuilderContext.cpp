#include "AbstractSyntaxTreeBuilderContext.h"

#include <algorithm>

#include "../code_generator/symbol_table.h"

#include "AbstractSyntaxTreeNode.h"
#include "Pointer.h"
#include "AssignmentExpressionList.h"
#include "Expression.h"
#include "LoopHeader.h"
#include "Declarator.h"
#include "DeclarationList.h"
#include "FormalArgument.h"
#include "ListCarrier.h"
#include "FunctionDeclarator.h"
#include "TypeSpecifier.h"

namespace ast {

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext() {
}

AbstractSyntaxTreeBuilderContext::~AbstractSyntaxTreeBuilderContext() {
}

void AbstractSyntaxTreeBuilderContext::pushTerminal(TerminalSymbol terminal) {
    terminalSymbols.push(terminal);
}

TerminalSymbol AbstractSyntaxTreeBuilderContext::popTerminal() {
    auto top = terminalSymbols.top();
    terminalSymbols.pop();
    return top;
}

void AbstractSyntaxTreeBuilderContext::pushTypeSpecifier(TypeSpecifier typeSpecifier) {
    typeSpecifiers.push(typeSpecifier);
}

TypeSpecifier AbstractSyntaxTreeBuilderContext::popTypeSpecifier() {
    auto typeSpecifier = typeSpecifiers.top();
    typeSpecifiers.pop();
    return typeSpecifier;
}

void AbstractSyntaxTreeBuilderContext::pushExpression(std::unique_ptr<Expression> expression) {
    expressionStack.push(std::move(expression));
}

std::unique_ptr<Expression> AbstractSyntaxTreeBuilderContext::popExpression() {
    auto expression = std::move(expressionStack.top());
    expressionStack.pop();
    return expression;
}

void AbstractSyntaxTreeBuilderContext::pushAssignmentExpressionList(std::unique_ptr<AssignmentExpressionList> assignmentExpressions) {
    assignmentExpressioListStack.push(std::move(assignmentExpressions));
}

std::unique_ptr<AssignmentExpressionList> AbstractSyntaxTreeBuilderContext::popAssignmentExpressionList() {
    auto assignmentExpressions = std::move(assignmentExpressioListStack.top());
    assignmentExpressioListStack.pop();
    return assignmentExpressions;
}

void AbstractSyntaxTreeBuilderContext::pushPointer(std::unique_ptr<Pointer> pointer) {
    pointerStack.push(std::move(pointer));
}

std::unique_ptr<Pointer> AbstractSyntaxTreeBuilderContext::popPointer() {
    auto pointer = std::move(pointerStack.top());
    pointerStack.pop();
    return pointer;
}

void AbstractSyntaxTreeBuilderContext::pushLoopHeader(std::unique_ptr<LoopHeader> loopHeader) {
    loopHeaderStack.push(std::move(loopHeader));
}

std::unique_ptr<LoopHeader> AbstractSyntaxTreeBuilderContext::popLoopHeader() {
    auto loopHeader = std::move(loopHeaderStack.top());
    loopHeaderStack.pop();
    return loopHeader;
}

void AbstractSyntaxTreeBuilderContext::pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> statement) {
    statementStack.push(std::move(statement));
}

std::unique_ptr<AbstractSyntaxTreeNode> AbstractSyntaxTreeBuilderContext::popStatement() {
    auto statement = std::move(statementStack.top());
    statementStack.pop();
    return statement;
}

void AbstractSyntaxTreeBuilderContext::pushDeclaration(std::unique_ptr<Declarator> declaration) {
    declarationStack.push(std::move(declaration));
}

std::unique_ptr<Declarator> AbstractSyntaxTreeBuilderContext::popDeclaration() {
    auto declaration = std::move(declarationStack.top());
    declarationStack.pop();
    return declaration;
}

void AbstractSyntaxTreeBuilderContext::pushDeclarationList(std::unique_ptr<DeclarationList> declarationList) {
    declarationListStack.push(std::move(declarationList));
}

std::unique_ptr<DeclarationList> AbstractSyntaxTreeBuilderContext::popDeclarationList() {
    auto declarationList = std::move(declarationListStack.top());
    declarationListStack.pop();
    return declarationList;
}

void AbstractSyntaxTreeBuilderContext::pushFunctionDeclaration(std::unique_ptr<FunctionDeclarator> declaration) {
    functionDeclarationStack.push(std::move(declaration));
}

std::unique_ptr<FunctionDeclarator> AbstractSyntaxTreeBuilderContext::popFunctionDeclaration() {
    auto declaration = std::move(functionDeclarationStack.top());
    functionDeclarationStack.pop();
    return declaration;
}

void AbstractSyntaxTreeBuilderContext::pushParameter(std::unique_ptr<FormalArgument> parameter) {
    parameterStack.push(std::move(parameter));
}

std::unique_ptr<FormalArgument> AbstractSyntaxTreeBuilderContext::popFormalArgument() {
    auto parameter = std::move(parameterStack.top());
    parameterStack.pop();
    return parameter;
}

void AbstractSyntaxTreeBuilderContext::pushFormalArguments(std::unique_ptr<std::vector<std::unique_ptr<FormalArgument>>> formalArguments) {
    formalArgumentLists.push(std::move(formalArguments));
}

std::unique_ptr<std::vector<std::unique_ptr<FormalArgument>>> AbstractSyntaxTreeBuilderContext::popFormalArguments() {
    auto formalArguments = std::move(formalArgumentLists.top());
    formalArgumentLists.pop();
    return formalArguments;
}

void AbstractSyntaxTreeBuilderContext::pushListCarrier(std::unique_ptr<ListCarrier> carrier) {
    listCarrierStack.push(std::move(carrier));
}

std::unique_ptr<ListCarrier> AbstractSyntaxTreeBuilderContext::popListCarrier() {
    auto carrier = std::move(listCarrierStack.top());
    listCarrierStack.pop();
    return carrier;
}

}
/* namespace ast */
