#include "AbstractSyntaxTreeBuilderContext.h"

#include <algorithm>
#include <vector>

#include "ListCarrier.h"
#include "FormalArgument.h"
#include "DeclarationList.h"
#include "LoopHeader.h"
#include "Pointer.h"
#include "ArgumentExpressionList.h"
#include "Expression.h"

namespace ast {

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext() {
}

AbstractSyntaxTreeBuilderContext::~AbstractSyntaxTreeBuilderContext() {
}

void AbstractSyntaxTreeBuilderContext::pushTerminal(TerminalSymbol terminal) {
    terminalSymbols.push(terminal);
}

TerminalSymbol AbstractSyntaxTreeBuilderContext::popTerminal() {
    auto terminal = terminalSymbols.top();
    terminalSymbols.pop();
    return terminal;
}

void AbstractSyntaxTreeBuilderContext::pushTypeSpecifier(TypeSpecifier typeSpecifier) {
    typeSpecifiers.push(typeSpecifier);
}

TypeSpecifier AbstractSyntaxTreeBuilderContext::popTypeSpecifier() {
    auto typeSpecifier = typeSpecifiers.top();
    typeSpecifiers.pop();
    return typeSpecifier;
}

void AbstractSyntaxTreeBuilderContext::pushStorageSpecifier(StorageSpecifier storageSpecifier) {
    storageSpecifiers.push(storageSpecifier);
}

StorageSpecifier AbstractSyntaxTreeBuilderContext::popStorageSpecifier() {
    auto storageSpecifier = storageSpecifiers.top();
    storageSpecifiers.pop();
    return storageSpecifier;
}

void AbstractSyntaxTreeBuilderContext::pushTypeQualifier(TypeQualifier typeQualifier) {
    typeQualifiers.push(typeQualifier);
}

TypeQualifier AbstractSyntaxTreeBuilderContext::popTypeQualifier() {
    auto typeQualifier = typeQualifiers.top();
    typeQualifiers.pop();
    return typeQualifier;
}

void AbstractSyntaxTreeBuilderContext::pushConstant(Constant constant) {
    constants.push(constant);
}

Constant AbstractSyntaxTreeBuilderContext::popConstant() {
    auto constant = constants.top();
    constants.pop();
    return constant;
}

void AbstractSyntaxTreeBuilderContext::pushExpression(std::unique_ptr<Expression> expression) {
    expressionStack.push(std::move(expression));
}

std::unique_ptr<Expression> AbstractSyntaxTreeBuilderContext::popExpression() {
    auto expression = std::move(expressionStack.top());
    expressionStack.pop();
    return expression;
}

void AbstractSyntaxTreeBuilderContext::pushAssignmentExpressionList(std::unique_ptr<ArgumentExpressionList> assignmentExpressions) {
    assignmentExpressioListStack.push(std::move(assignmentExpressions));
}

std::unique_ptr<ArgumentExpressionList> AbstractSyntaxTreeBuilderContext::popAssignmentExpressionList() {
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

void AbstractSyntaxTreeBuilderContext::pushDeclarator(std::unique_ptr<Declarator> declarator) {
    declarators.push(std::move(declarator));
}

std::unique_ptr<Declarator> AbstractSyntaxTreeBuilderContext::popDeclarator() {
    auto declarator = std::move(declarators.top());
    declarators.pop();
    return declarator;
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

void AbstractSyntaxTreeBuilderContext::pushFormalArgument(FormalArgument formalArgument) {
    formalArguments.push(std::move(formalArgument));
}

FormalArgument AbstractSyntaxTreeBuilderContext::popFormalArgument() {
    auto parameter = std::move(formalArguments.top());
    formalArguments.pop();
    return parameter;
}

void AbstractSyntaxTreeBuilderContext::pushFormalArguments(FormalArguments formalArguments) {
    formalArgumentLists.push(std::move(formalArguments));
}

FormalArguments AbstractSyntaxTreeBuilderContext::popFormalArguments() {
    auto formalArguments = std::move(formalArgumentLists.top());
    formalArgumentLists.pop();
    return formalArguments;
}

void AbstractSyntaxTreeBuilderContext::pushArgumentsDeclaration(std::pair<FormalArguments, bool> argumentsDeclaration) {
    argumentsDeclarations.push(std::move(argumentsDeclaration));
}

std::pair<FormalArguments, bool> AbstractSyntaxTreeBuilderContext::popArgumentsDeclaration() {
    auto argumentsDeclaration = std::move(argumentsDeclarations.top());
    argumentsDeclarations.pop();
    return argumentsDeclaration;
}

void AbstractSyntaxTreeBuilderContext::pushListCarrier(std::unique_ptr<ListCarrier> carrier) {
    listCarrierStack.push(std::move(carrier));
}

std::unique_ptr<ListCarrier> AbstractSyntaxTreeBuilderContext::popListCarrier() {
    auto carrier = std::move(listCarrierStack.top());
    listCarrierStack.pop();
    return carrier;
}

void AbstractSyntaxTreeBuilderContext::pushDeclarationSpecifiers(DeclarationSpecifiers declarationSpecifiers) {
    declarationSpecifiersStack.push(std::move(declarationSpecifiers));
}

DeclarationSpecifiers AbstractSyntaxTreeBuilderContext::popDeclarationSpecifiers() {
    auto declarationSpecifiers = std::move(declarationSpecifiersStack.top());
    declarationSpecifiersStack.pop();
    return declarationSpecifiers;
}

}
/* namespace ast */
