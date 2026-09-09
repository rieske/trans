#include "AbstractSyntaxTreeBuilderContext.h"

#include "Block.h"
#include "Expression.h"
#include "ExpressionStatement.h"
#include "Statement.h"
#include "util/Diagnostic.h"

#include <stdexcept>
#include <utility>

namespace ast {

namespace {

// Reached when a creator pops what its production never pushed, which means grammar.bnf
// and a CSNB creator disagree. A logic_error, not a user diagnostic.
[[noreturn]] void underflow(const char* what) {
    throw std::logic_error { std::string { "internal compiler error: parse stack underflow: " } + what };
}

template<typename Stack>
typename Stack::value_type& topOf(Stack& stack, const char* what) {
    if (stack.empty()) {
        underflow(what);
    }
    return stack.top();
}

template<typename Stack>
typename Stack::value_type popFrom(Stack& stack, const char* what) {
    auto value = std::move(topOf(stack, what));
    stack.pop();
    return value;
}

} // namespace

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
    return popFrom(terminalSymbols, "terminal");
}

void AbstractSyntaxTreeBuilderContext::pushTypeSpecifier(TypeSpecifier typeSpecifier) {
    typeSpecifiers.push(std::move(typeSpecifier));
}

bool AbstractSyntaxTreeBuilderContext::hasTypeSpecifier() const {
    return !typeSpecifiers.empty();
}

TypeSpecifier AbstractSyntaxTreeBuilderContext::popTypeSpecifier() {
    return popFrom(typeSpecifiers, "type specifier");
}

void AbstractSyntaxTreeBuilderContext::pushStorageSpecifier(StorageSpecifier storageSpecifier) {
    storageSpecifiers.push(storageSpecifier);
}

StorageSpecifier AbstractSyntaxTreeBuilderContext::popStorageSpecifier() {
    return popFrom(storageSpecifiers, "storage specifier");
}

void AbstractSyntaxTreeBuilderContext::pushFunctionSpecifier(FunctionSpecifier functionSpecifier) {
    functionSpecifiers.push(functionSpecifier);
}

FunctionSpecifier AbstractSyntaxTreeBuilderContext::popFunctionSpecifier() {
    return popFrom(functionSpecifiers, "function specifier");
}

void AbstractSyntaxTreeBuilderContext::pushTypeQualifier(type::Qualifier typeQualifier) {
    typeQualifiers.push(typeQualifier);
}

type::Qualifier AbstractSyntaxTreeBuilderContext::popTypeQualifier() {
    return popFrom(typeQualifiers, "type qualifier");
}

void AbstractSyntaxTreeBuilderContext::pushConstant(Constant constant) {
    constants.push(constant);
}

Constant AbstractSyntaxTreeBuilderContext::popConstant() {
    return popFrom(constants, "constant");
}

void AbstractSyntaxTreeBuilderContext::pushExpression(std::unique_ptr<Expression> expression) {
    expressionStack.push(std::move(expression));
}

std::unique_ptr<Expression> AbstractSyntaxTreeBuilderContext::popExpression() {
    return popFrom(expressionStack, "expression");
}

void AbstractSyntaxTreeBuilderContext::newActualArgumentsList(std::unique_ptr<Expression> argument) {
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::move(argument));
    actualArgumentLists.push(std::move(arguments));
}

void AbstractSyntaxTreeBuilderContext::addToActualArgumentsList(std::unique_ptr<Expression> argument) {
    topOf(actualArgumentLists, "actual argument list").push_back(std::move(argument));
}

std::vector<std::unique_ptr<Expression>> AbstractSyntaxTreeBuilderContext::popActualArgumentsList() {
    return popFrom(actualArgumentLists, "actual argument list");
}

void AbstractSyntaxTreeBuilderContext::newPointer(Pointer pointer) {
    pointerStack.push(std::vector<Pointer> { pointer });
}

void AbstractSyntaxTreeBuilderContext::pointerToPointer(Pointer pointer) {
    topOf(pointerStack, "pointer").push_back(pointer);
}

std::vector<Pointer> AbstractSyntaxTreeBuilderContext::popPointers() {
    return popFrom(pointerStack, "pointer");
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
    return popFrom(statementStack, "statement");
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
    return popStatement().takeBlock();
}

void AbstractSyntaxTreeBuilderContext::pushDirectDeclarator(std::unique_ptr<DirectDeclarator> declarator) {
    directDeclarators.push(std::move(declarator));
}

std::unique_ptr<DirectDeclarator> AbstractSyntaxTreeBuilderContext::popDirectDeclarator() {
    return popFrom(directDeclarators, "direct declarator");
}

void AbstractSyntaxTreeBuilderContext::pushDeclarator(std::unique_ptr<Declarator> declarator) {
    declarators.push(std::move(declarator));
}

std::unique_ptr<Declarator> AbstractSyntaxTreeBuilderContext::popDeclarator() {
    return popFrom(declarators, "declarator");
}

void AbstractSyntaxTreeBuilderContext::pushInitializedDeclarator(std::unique_ptr<InitializedDeclarator> initializedDeclarator) {
    initializedDeclarators.push(std::move(initializedDeclarator));
}

std::unique_ptr<InitializedDeclarator> AbstractSyntaxTreeBuilderContext::popInitializedDeclarator() {
    return popFrom(initializedDeclarators, "initialized declarator");
}

void AbstractSyntaxTreeBuilderContext::pushInitializedDeclarators(std::vector<std::unique_ptr<InitializedDeclarator> > declarators) {
    initializedDeclaratorLists.push(std::move(declarators));
}

std::vector<std::unique_ptr<InitializedDeclarator> > AbstractSyntaxTreeBuilderContext::popInitializedDeclarators() {
    return popFrom(initializedDeclaratorLists, "initialized declarator list");
}

void AbstractSyntaxTreeBuilderContext::pushDeclarationList(std::vector<std::unique_ptr<Declaration>> declarationList) {
    declarationLists.push(std::move(declarationList));
}

std::vector<std::unique_ptr<Declaration>> AbstractSyntaxTreeBuilderContext::popDeclarationList() {
    return popFrom(declarationLists, "declaration list");
}

void AbstractSyntaxTreeBuilderContext::pushFormalArgument(FormalArgument formalArgument) {
    formalArguments.push(std::move(formalArgument));
}

FormalArgument AbstractSyntaxTreeBuilderContext::popFormalArgument() {
    return popFrom(formalArguments, "formal argument");
}

void AbstractSyntaxTreeBuilderContext::pushFormalArguments(FormalArguments formalArguments) {
    formalArgumentLists.push(std::move(formalArguments));
}

FormalArguments AbstractSyntaxTreeBuilderContext::popFormalArguments() {
    return popFrom(formalArgumentLists, "formal argument list");
}

void AbstractSyntaxTreeBuilderContext::pushArgumentsDeclaration(std::pair<FormalArguments, bool> argumentsDeclaration) {
    argumentsDeclarations.push(std::move(argumentsDeclaration));
}

std::pair<FormalArguments, bool> AbstractSyntaxTreeBuilderContext::popArgumentsDeclaration() {
    return popFrom(argumentsDeclarations, "arguments declaration");
}

void AbstractSyntaxTreeBuilderContext::pushDeclarationSpecifiers(DeclarationSpecifiers declarationSpecifiers) {
    declarationSpecifiersStack.push(std::move(declarationSpecifiers));
}

DeclarationSpecifiers AbstractSyntaxTreeBuilderContext::popDeclarationSpecifiers() {
    return popFrom(declarationSpecifiersStack, "declaration specifiers");
}

void AbstractSyntaxTreeBuilderContext::pushDeclaration(std::unique_ptr<Declaration> declaration) {
    declarations.push(std::move(declaration));
}

void AbstractSyntaxTreeBuilderContext::newTypeQualifierList(type::Qualifier qualifier) {
    typeQualifierLists.push( { qualifier });
}

void AbstractSyntaxTreeBuilderContext::addToTypeQualifierList(type::Qualifier qualifier) {
    topOf(typeQualifierLists, "type qualifier list").push_back(qualifier);
}

std::vector<type::Qualifier> AbstractSyntaxTreeBuilderContext::popTypeQualifierList() {
    return popFrom(typeQualifierLists, "type qualifier list");
}

std::unique_ptr<Declaration> AbstractSyntaxTreeBuilderContext::popDeclaration() {
    return popFrom(declarations, "declaration");
}

void AbstractSyntaxTreeBuilderContext::newStatementList(BlockItem item) {
    std::vector<BlockItem> items;
    items.push_back(std::move(item));
    statementLists.push(std::move(items));
}

void AbstractSyntaxTreeBuilderContext::addToStatementList(BlockItem item) {
    topOf(statementLists, "statement list").push_back(std::move(item));
}

std::vector<BlockItem> AbstractSyntaxTreeBuilderContext::popStatementList() {
    return popFrom(statementLists, "statement list");
}

void AbstractSyntaxTreeBuilderContext::pushExternalDeclaration(ExternalDeclaration externalDeclaration) {
    externalDeclarations.push(std::move(externalDeclaration));
}

ExternalDeclaration AbstractSyntaxTreeBuilderContext::popExternalDeclaration() {
    return popFrom(externalDeclarations, "external declaration");
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
    return popFrom(isUnionStack, "struct or union");
}

void AbstractSyntaxTreeBuilderContext::newStructMemberList() {
    structMemberLists.push({});
}

void AbstractSyntaxTreeBuilderContext::addStructMember(std::string name, type::Type memberType,
        std::optional<int> bitWidth) {
    topOf(structMemberLists, "struct member list").members.push_back(
            type::MemberSpec { std::move(name), std::move(memberType), bitWidth });
}

void AbstractSyntaxTreeBuilderContext::addStructEnumerators(std::vector<Enumerator> enumerators) {
    auto& dest = topOf(structMemberLists, "struct member list").enumerators;
    dest.insert(dest.end(), enumerators.begin(), enumerators.end());
}

AbstractSyntaxTreeBuilderContext::RecordBody
AbstractSyntaxTreeBuilderContext::popStructMemberList() {
    return popFrom(structMemberLists, "struct member list");
}

void AbstractSyntaxTreeBuilderContext::newStructDeclaratorList() {
    structDeclaratorLists.push({});
}

void AbstractSyntaxTreeBuilderContext::addStructDeclarator(std::unique_ptr<Declarator> declarator,
        std::optional<int> bitWidth) {
    topOf(structDeclaratorLists, "struct declarator list").emplace_back(std::move(declarator), bitWidth);
}

std::vector<std::pair<std::unique_ptr<Declarator>, std::optional<int>>>
AbstractSyntaxTreeBuilderContext::takeStructDeclarators() {
    return std::exchange(topOf(structDeclaratorLists, "struct declarator list"), {});
}

void AbstractSyntaxTreeBuilderContext::popStructDeclaratorList() {
    if (!popFrom(structDeclaratorLists, "struct declarator list").empty()) {
        throw std::logic_error {
                "internal compiler error: struct declarator list closed with undrained declarators" };
    }
}

void AbstractSyntaxTreeBuilderContext::pushGenericAssociation(GenericAssociation association) {
    genericAssociations.push(std::move(association));
}

GenericAssociation AbstractSyntaxTreeBuilderContext::popGenericAssociation() {
    return popFrom(genericAssociations, "generic association");
}

void AbstractSyntaxTreeBuilderContext::newGenericAssocList(GenericAssociation association) {
    std::vector<GenericAssociation> associations;
    associations.push_back(std::move(association));
    genericAssocLists.push(std::move(associations));
}

void AbstractSyntaxTreeBuilderContext::addGenericAssociation(GenericAssociation association) {
    topOf(genericAssocLists, "generic association list").push_back(std::move(association));
}

std::vector<GenericAssociation> AbstractSyntaxTreeBuilderContext::popGenericAssocList() {
    return popFrom(genericAssocLists, "generic association list");
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
