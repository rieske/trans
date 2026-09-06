#include "AbstractSyntaxTreeBuilderContext.h"

#include "Block.h"
#include "Expression.h"
#include "ExpressionStatement.h"
#include "Statement.h"
#include "util/Diagnostic.h"

#include <utility>

namespace ast {

void AbstractSyntaxTreeBuilderContext::setSink(diag::Sink* sink) {
    sink_ = sink;
}

diag::Sink& AbstractSyntaxTreeBuilderContext::sink() const {
    if (!sink_) {
        throw std::logic_error { "missing diagnostic sink" };
    }
    return *sink_;
}

void AbstractSyntaxTreeBuilderContext::error(const translation_unit::Context& where, std::string message) {
    sink().error(where, std::move(message));
    failed_ = true;
}

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext(scanner::LexicalSession& session) :
        environment_ { session } {
}

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext(scanner::LexicalSession& session,
        ParseEnvironment& parent) :
        environment_ { session, parent } {
}

void AbstractSyntaxTreeBuilderContext::pushTerminal(TerminalSymbol terminal) {
    terminalSymbols.push(std::move(terminal));
}

TerminalSymbol AbstractSyntaxTreeBuilderContext::popTerminal() {
    TerminalSymbol terminal = std::move(terminalSymbols.top());
    terminalSymbols.pop();
    return terminal;
}

void AbstractSyntaxTreeBuilderContext::pushTypeSpecifier(TypeSpecifier typeSpecifier) {
    typeSpecifiers.push(typeSpecifier);
}

bool AbstractSyntaxTreeBuilderContext::hasTypeSpecifier() const {
    return !typeSpecifiers.empty();
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

void AbstractSyntaxTreeBuilderContext::pushTypeQualifier(type::Qualifier typeQualifier) {
    typeQualifiers.push(typeQualifier);
}

type::Qualifier AbstractSyntaxTreeBuilderContext::popTypeQualifier() {
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

void AbstractSyntaxTreeBuilderContext::newActualArgumentsList(std::unique_ptr<Expression> argument) {
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::move(argument));
    actualArgumentLists.push(std::move(arguments));
}

void AbstractSyntaxTreeBuilderContext::addToActualArgumentsList(std::unique_ptr<Expression> argument) {
    actualArgumentLists.top().push_back(std::move(argument));
}

std::vector<std::unique_ptr<Expression>> AbstractSyntaxTreeBuilderContext::popActualArgumentsList() {
    auto arguments = std::move(actualArgumentLists.top());
    actualArgumentLists.pop();
    return arguments;
}

void AbstractSyntaxTreeBuilderContext::newPointer(Pointer pointer) {
    pointerStack.push(std::vector<Pointer> { pointer });
}

void AbstractSyntaxTreeBuilderContext::pointerToPointer(Pointer pointer) {
    pointerStack.top().push_back(pointer);
}

std::vector<Pointer> AbstractSyntaxTreeBuilderContext::popPointers() {
    auto pointers = pointerStack.top();
    pointerStack.pop();
    return pointers;
}

void AbstractSyntaxTreeBuilderContext::pushStatement(std::unique_ptr<Statement> statement) {
    statementStack.push(BlockItem { std::move(statement) });
}

void AbstractSyntaxTreeBuilderContext::pushStatement(std::unique_ptr<Expression> expression) {
    statementStack.push(BlockItem { std::move(expression) });
}

void AbstractSyntaxTreeBuilderContext::pushStatement(std::unique_ptr<Declaration> declaration) {
    statementStack.push(BlockItem { std::move(declaration) });
}

BlockItem AbstractSyntaxTreeBuilderContext::popStatement() {
    auto item = std::move(statementStack.top());
    statementStack.pop();
    return item;
}

std::unique_ptr<Statement> AbstractSyntaxTreeBuilderContext::popAsStatement() {
    auto item = popStatement();
    if (auto statement = item.takeStatement()) {
        return statement;
    }
    if (auto expression = item.takeExpression()) {
        return std::make_unique<ExpressionStatement>(std::move(expression));
    }
    return nullptr;
}

std::unique_ptr<Block> AbstractSyntaxTreeBuilderContext::popBlock() {
    auto statement = popStatement().takeStatement();
    auto* block = statement ? statement->asBlock() : nullptr;
    if (!block) {
        return nullptr;
    }
    statement.release();
    return std::unique_ptr<Block> { block };
}

void AbstractSyntaxTreeBuilderContext::pushDirectDeclarator(std::unique_ptr<DirectDeclarator> declarator) {
    directDeclarators.push(std::move(declarator));
}

std::unique_ptr<DirectDeclarator> AbstractSyntaxTreeBuilderContext::popDirectDeclarator() {
    auto declarator = std::move(directDeclarators.top());
    directDeclarators.pop();
    return declarator;
}

void AbstractSyntaxTreeBuilderContext::pushDeclarator(std::unique_ptr<Declarator> declarator) {
    declarators.push(std::move(declarator));
}

std::unique_ptr<Declarator> AbstractSyntaxTreeBuilderContext::popDeclarator() {
    auto declarator = std::move(declarators.top());
    declarators.pop();
    return declarator;
}

void AbstractSyntaxTreeBuilderContext::pushInitializedDeclarator(std::unique_ptr<InitializedDeclarator> initializedDeclarator) {
    initializedDeclarators.push(std::move(initializedDeclarator));
}

std::unique_ptr<InitializedDeclarator> AbstractSyntaxTreeBuilderContext::popInitializedDeclarator() {
    auto initializedDeclarator = std::move(initializedDeclarators.top());
    initializedDeclarators.pop();
    return initializedDeclarator;
}

void AbstractSyntaxTreeBuilderContext::pushInitializedDeclarators(std::vector<std::unique_ptr<InitializedDeclarator> > declarators) {
    initializedDeclaratorLists.push(std::move(declarators));
}

std::vector<std::unique_ptr<InitializedDeclarator> > AbstractSyntaxTreeBuilderContext::popInitializedDeclarators() {
    auto declarators = std::move(initializedDeclaratorLists.top());
    initializedDeclaratorLists.pop();
    return declarators;
}

void AbstractSyntaxTreeBuilderContext::pushDeclarationList(std::vector<std::unique_ptr<Declaration>> declarationList) {
    declarationLists.push(std::move(declarationList));
}

std::vector<std::unique_ptr<Declaration>> AbstractSyntaxTreeBuilderContext::popDeclarationList() {
    auto declarationList = std::move(declarationLists.top());
    declarationLists.pop();
    return declarationList;
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

void AbstractSyntaxTreeBuilderContext::pushDeclarationSpecifiers(DeclarationSpecifiers declarationSpecifiers) {
    declarationSpecifiersStack.push(std::move(declarationSpecifiers));
}

DeclarationSpecifiers AbstractSyntaxTreeBuilderContext::popDeclarationSpecifiers() {
    auto declarationSpecifiers = std::move(declarationSpecifiersStack.top());
    declarationSpecifiersStack.pop();
    return declarationSpecifiers;
}

void AbstractSyntaxTreeBuilderContext::pushDeclaration(std::unique_ptr<Declaration> declaration) {
    declarations.push(std::move(declaration));
}

void AbstractSyntaxTreeBuilderContext::newTypeQualifierList(type::Qualifier qualifier) {
    typeQualifierLists.push( { qualifier });
}

void AbstractSyntaxTreeBuilderContext::addToTypeQualifierList(type::Qualifier qualifier) {
    typeQualifierLists.top().push_back(qualifier);
}

std::vector<type::Qualifier> AbstractSyntaxTreeBuilderContext::popTypeQualifierList() {
    auto qualifiers = typeQualifierLists.top();
    typeQualifierLists.pop();
    return qualifiers;
}

std::unique_ptr<Declaration> AbstractSyntaxTreeBuilderContext::popDeclaration() {
    auto declaration = std::move(declarations.top());
    declarations.pop();
    return declaration;
}

void AbstractSyntaxTreeBuilderContext::newStatementList(BlockItem item) {
    std::vector<BlockItem> items;
    items.push_back(std::move(item));
    statementLists.push(std::move(items));
}

void AbstractSyntaxTreeBuilderContext::addToStatementList(BlockItem item) {
    statementLists.top().push_back(std::move(item));
}

std::vector<BlockItem> AbstractSyntaxTreeBuilderContext::popStatementList() {
    auto items = std::move(statementLists.top());
    statementLists.pop();
    return items;
}

void AbstractSyntaxTreeBuilderContext::pushExternalDeclaration(ExternalDeclaration externalDeclaration) {
    externalDeclarations.push(std::move(externalDeclaration));
}

ExternalDeclaration AbstractSyntaxTreeBuilderContext::popExternalDeclaration() {
    auto externalDeclaration = std::move(externalDeclarations.top());
    externalDeclarations.pop();
    return externalDeclaration;
}

void AbstractSyntaxTreeBuilderContext::addToTranslationUnit(ExternalDeclaration externalDeclaration) {
    translationUnit.push_back(std::move(externalDeclaration));
}

std::vector<ExternalDeclaration> AbstractSyntaxTreeBuilderContext::popTranslationUnit() {
    return std::move(translationUnit);
}

void AbstractSyntaxTreeBuilderContext::pushIsUnion(bool isUnion) {
    isUnionStack.push(isUnion);
}

bool AbstractSyntaxTreeBuilderContext::popIsUnion() {
    bool v = isUnionStack.top();
    isUnionStack.pop();
    return v;
}

void AbstractSyntaxTreeBuilderContext::newStructMemberList() {
    structMemberLists.push({});
}

void AbstractSyntaxTreeBuilderContext::addStructMember(std::string name, type::Type memberType,
        std::optional<int> bitWidth) {
    structMemberLists.top().push_back(
            type::MemberSpec { std::move(name), std::move(memberType), bitWidth });
}

std::vector<type::MemberSpec> AbstractSyntaxTreeBuilderContext::popStructMemberList() {
    auto members = std::move(structMemberLists.top());
    structMemberLists.pop();
    return members;
}

void AbstractSyntaxTreeBuilderContext::addStructDeclarator(std::unique_ptr<Declarator> declarator,
        std::optional<int> bitWidth) {
    if (structDeclaratorLists.empty()) {
        structDeclaratorLists.push({});
    }
    structDeclaratorLists.top().emplace_back(std::move(declarator), bitWidth);
}

std::vector<std::pair<std::unique_ptr<Declarator>, std::optional<int>>>
AbstractSyntaxTreeBuilderContext::popStructDeclarators() {
    if (structDeclaratorLists.empty()) {
        return {};
    }
    auto declarators = std::move(structDeclaratorLists.top());
    structDeclaratorLists.pop();
    return declarators;
}

void AbstractSyntaxTreeBuilderContext::pushGenericAssociation(GenericAssociation association) {
    genericAssociations.push(std::move(association));
}

GenericAssociation AbstractSyntaxTreeBuilderContext::popGenericAssociation() {
    auto association = std::move(genericAssociations.top());
    genericAssociations.pop();
    return association;
}

void AbstractSyntaxTreeBuilderContext::newGenericAssocList(GenericAssociation association) {
    std::vector<GenericAssociation> associations;
    associations.push_back(std::move(association));
    genericAssocLists.push(std::move(associations));
}

void AbstractSyntaxTreeBuilderContext::addGenericAssociation(GenericAssociation association) {
    genericAssocLists.top().push_back(std::move(association));
}

std::vector<GenericAssociation> AbstractSyntaxTreeBuilderContext::popGenericAssocList() {
    auto associations = std::move(genericAssocLists.top());
    genericAssocLists.pop();
    return associations;
}

void AbstractSyntaxTreeBuilderContext::newInitializerList() {
    initializerLists.push({});
}

void AbstractSyntaxTreeBuilderContext::addInitializerElement(InitializerElement element) {
    if (initializerLists.empty()) {
        newInitializerList();
    }
    initializerLists.top().push_back(std::move(element));
}

std::vector<InitializerElement> AbstractSyntaxTreeBuilderContext::popInitializerList() {
    if (initializerLists.empty()) {
        return {};
    }
    auto list = std::move(initializerLists.top());
    initializerLists.pop();
    return list;
}

void AbstractSyntaxTreeBuilderContext::pushMemberDesignator(std::string memberName) {
    std::vector<DesignatorStep> steps;
    steps.push_back(DesignatorStep::member(std::move(memberName)));
    pendingDesignators.push(std::move(steps));
}

void AbstractSyntaxTreeBuilderContext::pushArrayIndexDesignator(std::unique_ptr<Expression> indexExpression) {
    std::vector<DesignatorStep> steps;
    steps.push_back(DesignatorStep::indexWithExpression(std::move(indexExpression)));
    pendingDesignators.push(std::move(steps));
}

void AbstractSyntaxTreeBuilderContext::pushPendingDesignator(std::vector<DesignatorStep> steps) {
    pendingDesignators.push(std::move(steps));
}

void AbstractSyntaxTreeBuilderContext::takePendingDesignator(std::vector<DesignatorStep>& steps) {
    if (pendingDesignators.empty()) {
        steps.clear();
        return;
    }
    steps = std::move(pendingDesignators.top());
    pendingDesignators.pop();
}

} // namespace ast
