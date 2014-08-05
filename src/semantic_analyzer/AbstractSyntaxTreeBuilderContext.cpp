#include "AbstractSyntaxTreeBuilderContext.h"

#include <algorithm>

#include "../code_generator/symbol_table.h"

#include "AbstractSyntaxTreeNode.h"
#include "Pointer.h"
#include "AssignmentExpressionList.h"
#include "Expression.h"
#include "LoopHeader.h"
#include "Declaration.h"
#include "DeclarationList.h"
#include "ParameterList.h"
#include "ParameterDeclaration.h"
#include "ListCarrier.h"
#include "FunctionDeclaration.h"
#include "TypeSpecifier.h"

namespace semantic_analyzer {

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext() :
        currentScope { new SymbolTable() } {
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

    // FIXME: incorporate scope data into the AST
    if (terminal.value == "{") {
        innerScope();
        for (const auto declaredParam : declaredParams) {
            currentScope->insertParam(declaredParam->getPlace()->getName(), declaredParam->getPlace()->getBasicType(),
                    declaredParam->getPlace()->getExtendedType(), currentLine);
        }
        declaredParams.clear();
    } else if (terminal.value == "}") {
        outerScope();
    }
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

void AbstractSyntaxTreeBuilderContext::pushDeclaration(std::unique_ptr<Declaration> declaration) {
    declarationStack.push(std::move(declaration));
}

std::unique_ptr<Declaration> AbstractSyntaxTreeBuilderContext::popDeclaration() {
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

void AbstractSyntaxTreeBuilderContext::pushFunctionDeclaration(std::unique_ptr<FunctionDeclaration> declaration) {
    declaredParams = declaration->getParams();
    functionDeclarationStack.push(std::move(declaration));
}

std::unique_ptr<FunctionDeclaration> AbstractSyntaxTreeBuilderContext::popFunctionDeclaration() {
    auto declaration = std::move(functionDeclarationStack.top());
    functionDeclarationStack.pop();
    return declaration;
}

void AbstractSyntaxTreeBuilderContext::pushParameter(std::unique_ptr<ParameterDeclaration> parameter) {
    parameterStack.push(std::move(parameter));
}

std::unique_ptr<ParameterDeclaration> AbstractSyntaxTreeBuilderContext::popParameter() {
    auto parameter = std::move(parameterStack.top());
    parameterStack.pop();
    return parameter;
}

void AbstractSyntaxTreeBuilderContext::pushParameterList(std::unique_ptr<ParameterList> parameters) {
    parameterListStack.push(std::move(parameters));
}

std::unique_ptr<ParameterList> AbstractSyntaxTreeBuilderContext::popParameterList() {
    auto parameterList = std::move(parameterListStack.top());
    parameterListStack.pop();
    return parameterList;
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
/* namespace semantic_analyzer */
