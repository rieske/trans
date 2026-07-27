#include "AbstractSyntaxTreeBuilderContext.h"

namespace ast {

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext() {
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

void AbstractSyntaxTreeBuilderContext::addStructMember(std::string name, type::Type memberType) {
    structMemberLists.top().emplace_back(std::move(name), std::move(memberType));
}

std::vector<std::pair<std::string, type::Type>> AbstractSyntaxTreeBuilderContext::popStructMemberList() {
    auto members = std::move(structMemberLists.top());
    structMemberLists.pop();
    return members;
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
    auto declarators = std::move(structDeclaratorLists.top());
    structDeclaratorLists.pop();
    return declarators;
}

type::Type AbstractSyntaxTreeBuilderContext::ensureStructTag(const std::string& tag) {
    auto it = structTags.find(tag);
    if (it != structTags.end()) {
        return it->second;
    }
    // Incomplete placeholder until a defining body completes the tag.
    type::Type incomplete = type::incompleteStructure();
    structTags.insert({ tag, incomplete });
    return incomplete;
}

void AbstractSyntaxTreeBuilderContext::completeStructTag(const std::string& tag, type::Type completeType) {
    auto it = structTags.find(tag);
    if (it != structTags.end() && it->second.isStructure()) {
        // In-place: keep the incomplete Type's shared StructBody identity so
        // pointer-to-tag types captured during the body (struct Node *next)
        // observe the completed layout.
        std::vector<std::pair<std::string, type::Type>> members;
        for (int i = 0; i < completeType.memberCount(); ++i) {
            std::string name;
            type::Type memberType = type::voidType();
            int offset = 0;
            if (completeType.memberAt(i, name, memberType, offset)) {
                members.emplace_back(std::move(name), std::move(memberType));
            }
        }
        type::completeStructure(it->second, members);
        return;
    }
    structTags.insert_or_assign(tag, std::move(completeType));
}

bool AbstractSyntaxTreeBuilderContext::hasStructTag(const std::string& tag) const {
    return structTags.find(tag) != structTags.end();
}

type::Type AbstractSyntaxTreeBuilderContext::lookupStructTag(const std::string& tag) const {
    return structTags.at(tag);
}

void AbstractSyntaxTreeBuilderContext::newInitializerList() {
    initializerLists.push({});
}

void AbstractSyntaxTreeBuilderContext::addInitializerElement(std::unique_ptr<Expression> expression) {
    if (initializerLists.empty()) {
        newInitializerList();
    }
    initializerLists.top().push_back(std::move(expression));
}

std::vector<std::unique_ptr<Expression>> AbstractSyntaxTreeBuilderContext::popInitializerList() {
    if (initializerLists.empty()) {
        return {};
    }
    auto list = std::move(initializerLists.top());
    initializerLists.pop();
    return list;
}

} // namespace ast
