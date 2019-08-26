#ifndef ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_
#define ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_

#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include "Constant.h"
#include "DeclarationSpecifiers.h"
#include "FormalArgument.h"
#include "FunctionDeclarator.h"
#include "Pointer.h"
#include "StorageSpecifier.h"
#include "TerminalSymbol.h"
#include "../types/TypeQualifier.h"
#include "TypeSpecifier.h"
#include "Declaration.h"
#include "InitializedDeclarator.h"
#include "LoopHeader.h"

namespace ast {

class AbstractSyntaxTreeBuilderContext {
public:
    AbstractSyntaxTreeBuilderContext();
    virtual ~AbstractSyntaxTreeBuilderContext() = default;

    void pushTerminal(TerminalSymbol terminal);
    TerminalSymbol popTerminal();

    void pushTypeSpecifier(TypeSpecifier typeSpecifier);
    TypeSpecifier popTypeSpecifier();

    void pushStorageSpecifier(StorageSpecifier storageSpecifier);
    StorageSpecifier popStorageSpecifier();

    void pushTypeQualifier(TypeQualifier typeQualifier);
    TypeQualifier popTypeQualifier();

    void newTypeQualifierList(TypeQualifier qualifier);
    void addToTypeQualifierList(TypeQualifier qualifier);
    std::vector<TypeQualifier> popTypeQualifierList();

    void pushConstant(Constant constant);
    Constant popConstant();

    void pushExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Expression> popExpression();

    void newActualArgumentsList(std::unique_ptr<Expression> argument);
    void addToActualArgumentsList(std::unique_ptr<Expression> argument);
    std::vector<std::unique_ptr<Expression>> popActualArgumentsList();

    void newPointer(Pointer pointer);
    void pointerToPointer(Pointer pointer);
    std::vector<Pointer> popPointers();

    void pushLoopHeader(std::unique_ptr<LoopHeader> loopHeader);
    std::unique_ptr<LoopHeader> popLoopHeader();

    void pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> statement);
    std::unique_ptr<AbstractSyntaxTreeNode> popStatement();

    void pushDirectDeclarator(std::unique_ptr<DirectDeclarator> declarator);
    std::unique_ptr<DirectDeclarator> popDirectDeclarator();

    void pushDeclarator(std::unique_ptr<Declarator> declarator);
    std::unique_ptr<Declarator> popDeclarator();

    void pushInitializedDeclarator(std::unique_ptr<InitializedDeclarator> initializedDeclarator);
    std::unique_ptr<InitializedDeclarator> popInitializedDeclarator();

    void pushInitializedDeclarators(std::vector<std::unique_ptr<InitializedDeclarator>> declarators);
    std::vector<std::unique_ptr<InitializedDeclarator>> popInitializedDeclarators();

    void pushDeclarationList(std::vector<std::unique_ptr<Declaration>> declarationList);
    std::vector<std::unique_ptr<Declaration>> popDeclarationList();

    void pushFormalArgument(FormalArgument formalArgument);
    FormalArgument popFormalArgument();

    void pushFormalArguments(FormalArguments formalArguments);
    FormalArguments popFormalArguments();

    void pushArgumentsDeclaration(std::pair<FormalArguments, bool> argumentsDeclaration);
    std::pair<FormalArguments, bool> popArgumentsDeclaration();

    void pushDeclarationSpecifiers(DeclarationSpecifiers declarationSpecifiers);
    DeclarationSpecifiers popDeclarationSpecifiers();

    void pushDeclaration(std::unique_ptr<Declaration> declaration);
    std::unique_ptr<Declaration> popDeclaration();

    void newStatementList(std::unique_ptr<AbstractSyntaxTreeNode> statement);
    void addToStatementList(std::unique_ptr<AbstractSyntaxTreeNode> statement);
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> popStatementList();

    void pushExternalDeclaration(std::unique_ptr<AbstractSyntaxTreeNode> externalDeclaration);
    std::unique_ptr<AbstractSyntaxTreeNode> popExternalDeclaration();

    void addToTranslationUnit(std::unique_ptr<AbstractSyntaxTreeNode> externalDeclaration);
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> popTranslationUnit();

private:
    std::stack<TerminalSymbol> terminalSymbols;

    std::stack<TypeSpecifier> typeSpecifiers;
    std::stack<StorageSpecifier> storageSpecifiers;
    std::stack<TypeQualifier> typeQualifiers;
    std::stack<std::vector<TypeQualifier>> typeQualifierLists;
    std::stack<Constant> constants;

    std::stack<DeclarationSpecifiers> declarationSpecifiersStack;

    std::stack<std::unique_ptr<Expression>> expressionStack;
    std::stack<std::vector<std::unique_ptr<Expression>>>actualArgumentLists;
    std::stack<std::vector<Pointer>> pointerStack;
    std::stack<std::unique_ptr<LoopHeader>> loopHeaderStack;
    std::stack<std::unique_ptr<AbstractSyntaxTreeNode>> statementStack;
    std::stack<std::unique_ptr<DirectDeclarator>> directDeclarators;
    std::stack<std::unique_ptr<Declarator>> declarators;
    std::stack<std::unique_ptr<InitializedDeclarator>> initializedDeclarators;
    std::stack<std::vector<std::unique_ptr<InitializedDeclarator>>>initializedDeclaratorLists;
    std::stack<FormalArgument> formalArguments;
    std::stack<FormalArguments> formalArgumentLists;
    std::stack<std::pair<FormalArguments, bool>> argumentsDeclarations;
    std::stack<std::unique_ptr<Declaration>> declarations;
    std::stack<std::vector<std::unique_ptr<Declaration>>> declarationLists;
    std::stack<std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>> statementLists;
    std::stack<std::unique_ptr<AbstractSyntaxTreeNode>> externalDeclarations;
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit;
};

}
/* namespace ast */

#endif /* ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_ */
