#ifndef ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_
#define ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_

#include <memory>
#include <stack>
#include <utility>

#include "Constant.h"
#include "DeclarationSpecifiers.h"
#include "FormalArgument.h"
#include "FunctionDeclarator.h"
#include "StorageSpecifier.h"
#include "TerminalSymbol.h"
#include "TypeQualifier.h"
#include "TypeSpecifier.h"

namespace ast {

class DeclarationSpecifiers;
class Expression;
class ArgumentExpressionList;
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

    void pushStorageSpecifier(StorageSpecifier storageSpecifier);
    StorageSpecifier popStorageSpecifier();

    void pushTypeQualifier(TypeQualifier typeQualifier);
    TypeQualifier popTypeQualifier();

    void pushConstant(Constant constant);
    Constant popConstant();

    void pushExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Expression> popExpression();

    void pushAssignmentExpressionList(std::unique_ptr<ArgumentExpressionList> assignmentExpressions);
    std::unique_ptr<ArgumentExpressionList> popAssignmentExpressionList();

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

    void pushFormalArgument(FormalArgument formalArgument);
    FormalArgument popFormalArgument();

    void pushFormalArguments(FormalArguments formalArguments);
    FormalArguments popFormalArguments();

    void pushArgumentsDeclaration(std::pair<FormalArguments, bool> argumentsDeclaration);
    std::pair<FormalArguments, bool> popArgumentsDeclaration();

    void pushListCarrier(std::unique_ptr<ListCarrier> carrier);
    std::unique_ptr<ListCarrier> popListCarrier();

    void pushDeclarationSpecifiers(DeclarationSpecifiers declarationSpecifiers);
    DeclarationSpecifiers popDeclarationSpecifiers();

private:
    std::stack<TerminalSymbol> terminalSymbols;

    std::stack<TypeSpecifier> typeSpecifiers;
    std::stack<StorageSpecifier> storageSpecifiers;
    std::stack<TypeQualifier> typeQualifiers;
    std::stack<Constant> constants;

    std::stack<DeclarationSpecifiers> declarationSpecifiersStack;

    std::stack<std::unique_ptr<Expression>> expressionStack;
    std::stack<std::unique_ptr<ArgumentExpressionList>> assignmentExpressioListStack;
    std::stack<std::unique_ptr<Pointer>> pointerStack;
    std::stack<std::unique_ptr<LoopHeader>> loopHeaderStack;
    std::stack<std::unique_ptr<AbstractSyntaxTreeNode>> statementStack;
    std::stack<std::unique_ptr<Declarator>> declarators;
    std::stack<std::unique_ptr<DeclarationList>> declarationListStack;
    std::stack<std::unique_ptr<FunctionDeclarator>> functionDeclarationStack;
    std::stack<FormalArgument> formalArguments;
    std::stack<FormalArguments> formalArgumentLists;
    std::stack<std::pair<FormalArguments, bool>> argumentsDeclarations;
    std::stack<std::unique_ptr<ListCarrier>> listCarrierStack;
};

}
/* namespace ast */

#endif /* ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_ */
