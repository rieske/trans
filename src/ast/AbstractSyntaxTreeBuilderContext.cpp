#include "AbstractSyntaxTreeBuilderContext.h"

namespace ast {

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext() = default;

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

bool AbstractSyntaxTreeBuilderContext::hasPointers() const {
    return !pointerStack.empty();
}

std::vector<Pointer> AbstractSyntaxTreeBuilderContext::popPointers() {
    auto pointers = pointerStack.top();
    pointerStack.pop();
    return pointers;
}

void AbstractSyntaxTreeBuilderContext::pushAbstractPointers(std::vector<Pointer> pointers) {
    abstractPointerStack.push(std::move(pointers));
}

bool AbstractSyntaxTreeBuilderContext::hasAbstractPointers() const {
    return !abstractPointerStack.empty();
}

std::vector<Pointer> AbstractSyntaxTreeBuilderContext::popAbstractPointers() {
    auto pointers = abstractPointerStack.top();
    abstractPointerStack.pop();
    return pointers;
}

void AbstractSyntaxTreeBuilderContext::pushAbstractArraySize(std::unique_ptr<Expression> sizeExpression) {
    abstractArraySizes.push(std::move(sizeExpression));
}

bool AbstractSyntaxTreeBuilderContext::hasAbstractArray() const {
    return !abstractArraySizes.empty();
}

std::unique_ptr<Expression> AbstractSyntaxTreeBuilderContext::popAbstractArraySize() {
    auto size = std::move(abstractArraySizes.top());
    abstractArraySizes.pop();
    return size;
}

void AbstractSyntaxTreeBuilderContext::pushTypeofOperand(std::unique_ptr<Expression> expression) {
    typeofOperands.push(std::move(expression));
}

bool AbstractSyntaxTreeBuilderContext::hasTypeofOperand() const {
    return !typeofOperands.empty();
}

std::unique_ptr<Expression> AbstractSyntaxTreeBuilderContext::popTypeofOperand() {
    auto expr = std::move(typeofOperands.top());
    typeofOperands.pop();
    return expr;
}

void AbstractSyntaxTreeBuilderContext::pushGenericAssociation(GenericAssociation association) {
    genericAssociations.push(std::move(association));
}

AbstractSyntaxTreeBuilderContext::GenericAssociation AbstractSyntaxTreeBuilderContext::popGenericAssociation() {
    auto association = std::move(genericAssociations.top());
    genericAssociations.pop();
    return association;
}

void AbstractSyntaxTreeBuilderContext::pushGenericAssociationList(std::vector<GenericAssociation> list) {
    genericAssociationLists.push(std::move(list));
}

std::vector<AbstractSyntaxTreeBuilderContext::GenericAssociation>
AbstractSyntaxTreeBuilderContext::popGenericAssociationList() {
    auto list = std::move(genericAssociationLists.top());
    genericAssociationLists.pop();
    return list;
}

void AbstractSyntaxTreeBuilderContext::pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> statement) {
    statementStack.push(std::move(statement));
}

std::unique_ptr<AbstractSyntaxTreeNode> AbstractSyntaxTreeBuilderContext::popStatement() {
    auto statement = std::move(statementStack.top());
    statementStack.pop();
    return statement;
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

void AbstractSyntaxTreeBuilderContext::newStatementList(std::unique_ptr<AbstractSyntaxTreeNode> statement) {
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> statements;
    statements.push_back(std::move(statement));
    statementLists.push(std::move(statements));
}

void AbstractSyntaxTreeBuilderContext::addToStatementList(std::unique_ptr<AbstractSyntaxTreeNode> statement) {
    statementLists.top().push_back(std::move(statement));
}

std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> AbstractSyntaxTreeBuilderContext::popStatementList() {
    auto statements = std::move(statementLists.top());
    statementLists.pop();
    return statements;
}

void AbstractSyntaxTreeBuilderContext::pushExternalDeclaration(std::unique_ptr<AbstractSyntaxTreeNode> externalDeclaration) {
    externalDeclarations.push(std::move(externalDeclaration));
}

std::unique_ptr<AbstractSyntaxTreeNode> AbstractSyntaxTreeBuilderContext::popExternalDeclaration() {
    auto externalDeclaration = std::move(externalDeclarations.top());
    externalDeclarations.pop();
    return externalDeclaration;
}

void AbstractSyntaxTreeBuilderContext::addToTranslationUnit(std::unique_ptr<AbstractSyntaxTreeNode> externalDeclaration) {
    translationUnit.push_back(std::move(externalDeclaration));
}

std::vector<std::unique_ptr<AbstractSyntaxTreeNode> > AbstractSyntaxTreeBuilderContext::popTranslationUnit() {
    return std::move(translationUnit);
}

void AbstractSyntaxTreeBuilderContext::newStructMemberList() {
    structMemberLists.push({});
    pendingMemberDeclaratorLists.push({});
}

void AbstractSyntaxTreeBuilderContext::addStructMember(std::string name, type::Type type) {
    if (structMemberLists.empty()) {
        structMemberLists.push({});
    }
    structMemberLists.top().emplace_back(std::move(name), std::move(type));
}

void AbstractSyntaxTreeBuilderContext::addStructMembersFromList(std::vector<std::pair<std::string, type::Type>> members) {
    for (auto& m : members) {
        structMemberLists.top().push_back(std::move(m));
    }
}

std::vector<std::pair<std::string, type::Type>> AbstractSyntaxTreeBuilderContext::popStructMemberList() {
    auto list = std::move(structMemberLists.top());
    structMemberLists.pop();
    return list;
}

void AbstractSyntaxTreeBuilderContext::pushStructMemberList(std::vector<std::pair<std::string, type::Type>> members) {
    structMemberLists.push(std::move(members));
}

void AbstractSyntaxTreeBuilderContext::addPendingMemberDeclarator(
        std::string name, type::Type baseType, std::unique_ptr<Declarator> declarator) {
    if (pendingMemberDeclaratorLists.empty()) {
        pendingMemberDeclaratorLists.push({});
    }
    PendingMemberDeclarator pending { std::move(name), std::move(baseType), std::move(declarator) };
    pendingMemberDeclaratorLists.top().push_back(std::move(pending));
}

std::vector<AbstractSyntaxTreeBuilderContext::PendingMemberDeclarator>
AbstractSyntaxTreeBuilderContext::popPendingMemberDeclarators() {
    if (pendingMemberDeclaratorLists.empty()) {
        return {};
    }
    auto list = std::move(pendingMemberDeclaratorLists.top());
    pendingMemberDeclaratorLists.pop();
    return list;
}



void AbstractSyntaxTreeBuilderContext::addStructDeclarator(std::unique_ptr<Declarator> declarator) {
    if (structDeclaratorLists.empty()) {
        structDeclaratorLists.push({});
    }
    structDeclaratorLists.top().push_back(std::move(declarator));
}

std::vector<std::unique_ptr<Declarator>> AbstractSyntaxTreeBuilderContext::popStructDeclarators() {
    if (structDeclaratorLists.empty()) {
        return {};
    }
    auto list = std::move(structDeclaratorLists.top());
    structDeclaratorLists.pop();
    return list;
}

void AbstractSyntaxTreeBuilderContext::pushIsUnion(bool isUnion) {
    isUnionStack.push(isUnion);
}

bool AbstractSyntaxTreeBuilderContext::popIsUnion() {
    if (isUnionStack.empty()) {
        return false;
    }
    bool value = isUnionStack.top();
    isUnionStack.pop();
    return value;
}

void AbstractSyntaxTreeBuilderContext::pushMemberDesignator(std::string memberName) {
    PendingDesignator d;
    d.memberPath.push_back(std::move(memberName));
    pendingDesignators.push(std::move(d));
}

void AbstractSyntaxTreeBuilderContext::pushArrayIndexDesignator(std::unique_ptr<Expression> indexExpression) {
    PendingDesignator d;
    long index = 0;
    if (indexExpression && indexExpression->evaluateConstant(index)) {
        d.arrayIndex = index;
    } else {
        d.arrayIndex = 0;
    }
    pendingDesignators.push(std::move(d));
}

void AbstractSyntaxTreeBuilderContext::pushPendingDesignator(std::vector<std::string> memberPath,
        std::optional<long> arrayIndex) {
    PendingDesignator d;
    d.memberPath = std::move(memberPath);
    d.arrayIndex = arrayIndex;
    pendingDesignators.push(std::move(d));
}

bool AbstractSyntaxTreeBuilderContext::hasPendingDesignator() const {
    return !pendingDesignators.empty();
}

void AbstractSyntaxTreeBuilderContext::takePendingDesignator(std::vector<std::string>& memberPath,
        std::optional<long>& arrayIndex) {
    if (pendingDesignators.empty()) {
        memberPath.clear();
        arrayIndex.reset();
        return;
    }
    PendingDesignator d = std::move(pendingDesignators.top());
    pendingDesignators.pop();
    memberPath = std::move(d.memberPath);
    arrayIndex = d.arrayIndex;
}

} // namespace ast
