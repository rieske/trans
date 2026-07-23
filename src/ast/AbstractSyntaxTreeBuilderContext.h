#ifndef ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_
#define ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_

#include <memory>
#include <stack>
#include <vector>
#include <utility>
#include <optional>

#include "Constant.h"
#include "DeclarationSpecifiers.h"
#include "FormalArgument.h"
#include "FunctionDeclarator.h"
#include "ParseEnvironment.h"
#include "Pointer.h"
#include "StorageSpecifier.h"
#include "TerminalSymbol.h"
#include "TypeSpecifier.h"
#include "Declaration.h"
#include "InitializedDeclarator.h"
#include "PendingArrayMemberStore.h"

namespace ast {

// Bottom-up reduction stacks for AST construction, plus a ParseEnvironment for
// tags/typedefs/enums/pending ARRAY_SIZE members (not reduction state).
class AbstractSyntaxTreeBuilderContext {
public:
    AbstractSyntaxTreeBuilderContext();
    virtual ~AbstractSyntaxTreeBuilderContext() = default;

    ParseEnvironment& environment() { return environment_; }
    const ParseEnvironment& environment() const { return environment_; }

    void pushTerminal(TerminalSymbol terminal);
    TerminalSymbol popTerminal();

    void pushTypeSpecifier(TypeSpecifier typeSpecifier);
    bool hasTypeSpecifier() const;
    TypeSpecifier popTypeSpecifier();

    void pushStorageSpecifier(StorageSpecifier storageSpecifier);
    StorageSpecifier popStorageSpecifier();

    void pushTypeQualifier(type::Qualifier typeQualifier);
    type::Qualifier popTypeQualifier();

    void newTypeQualifierList(type::Qualifier qualifier);
    void addToTypeQualifierList(type::Qualifier qualifier);
    std::vector<type::Qualifier> popTypeQualifierList();

    void pushConstant(Constant constant);
    Constant popConstant();

    void pushExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Expression> popExpression();

    void newActualArgumentsList(std::unique_ptr<Expression> argument);
    void addToActualArgumentsList(std::unique_ptr<Expression> argument);
    std::vector<std::unique_ptr<Expression>> popActualArgumentsList();

    void newPointer(Pointer pointer);
    void pointerToPointer(Pointer pointer);
    bool hasPointers() const;
    std::vector<Pointer> popPointers();

    // Pointers belonging to an abstract declarator (not an outer named declarator).
    // Sealing isolates e.g. the '*' in `char *f(char *)` from the outer function '*'.
    void pushAbstractPointers(std::vector<Pointer> pointers);
    bool hasAbstractPointers() const;
    std::vector<Pointer> popAbstractPointers();

    // Abstract array declarators (e.g. int [2], char *[]) for parameter decay.
    void pushAbstractArraySize(std::unique_ptr<Expression> sizeExpression);
    bool hasAbstractArray() const;
    std::unique_ptr<Expression> popAbstractArraySize();

    // __typeof__(expr) type_name: keep the operand for offsetof pointer form.
    void pushTypeofOperand(std::unique_ptr<Expression> expression);
    bool hasTypeofOperand() const;
    std::unique_ptr<Expression> popTypeofOperand();

    // _Generic associations: isDefault + selected expression (product contract:
    // pick default if present, else last association).
    struct GenericAssociation {
        bool isDefault { false };
        std::unique_ptr<Expression> expression;
    };
    void pushGenericAssociation(GenericAssociation association);
    GenericAssociation popGenericAssociation();
    void pushGenericAssociationList(std::vector<GenericAssociation> list);
    std::vector<GenericAssociation> popGenericAssociationList();

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

    void newStructMemberList();
    void addStructMember(std::string name, type::Type type);
    void addStructMembersFromList(std::vector<std::pair<std::string, type::Type>> members);
    std::vector<std::pair<std::string, type::Type>> popStructMemberList();
    void pushStructMemberList(std::vector<std::pair<std::string, type::Type>> members);

    // Keep member declarators until the struct/union is completed so ARRAY_SIZE
    // bounds can be re-folded during semantic analysis.
    struct PendingMemberDeclarator {
        std::string name;
        type::Type baseType;
        std::unique_ptr<Declarator> declarator;
    };
    void addPendingMemberDeclarator(std::string name, type::Type baseType, std::unique_ptr<Declarator> declarator);
    std::vector<PendingMemberDeclarator> popPendingMemberDeclarators();

    void addStructDeclarator(std::unique_ptr<Declarator> declarator);
    std::vector<std::unique_ptr<Declarator>> popStructDeclarators();

    // Whether the most recently reduced struct_or_union was a union (reduction stack).
    void pushIsUnion(bool isUnion);
    bool popIsUnion();

    // C99 designators for brace initializers (.member / [index] / nested .a.b).
    void pushMemberDesignator(std::string memberName);
    void pushArrayIndexDesignator(std::unique_ptr<Expression> indexExpression);
    // Push a fully-built designator (used when merging nested designator_list segments).
    void pushPendingDesignator(std::vector<std::string> memberPath, std::optional<long> arrayIndex);
    bool hasPendingDesignator() const;
    // memberPath is .a.b.c names; arrayIndex is a trailing or sole [n].
    void takePendingDesignator(std::vector<std::string>& memberPath, std::optional<long>& arrayIndex);

private:
    ParseEnvironment environment_;

    std::stack<TerminalSymbol> terminalSymbols;

    std::stack<TypeSpecifier> typeSpecifiers;
    std::stack<StorageSpecifier> storageSpecifiers;
    std::stack<type::Qualifier> typeQualifiers;
    std::stack<std::vector<type::Qualifier>> typeQualifierLists;
    std::stack<Constant> constants;

    std::stack<DeclarationSpecifiers> declarationSpecifiersStack;

    std::stack<std::unique_ptr<Expression>> expressionStack;
    std::stack<std::vector<std::unique_ptr<Expression>>> actualArgumentLists;
    std::stack<std::vector<Pointer>> pointerStack;
    std::stack<std::vector<Pointer>> abstractPointerStack;
    // Top entry is optional size expression for the most recent abstract array `[]` / `[N]`.
    std::stack<std::unique_ptr<Expression>> abstractArraySizes;
    std::stack<std::unique_ptr<Expression>> typeofOperands;
    std::stack<GenericAssociation> genericAssociations;
    std::stack<std::vector<GenericAssociation>> genericAssociationLists;
    std::stack<std::unique_ptr<AbstractSyntaxTreeNode>> statementStack;
    std::stack<std::unique_ptr<DirectDeclarator>> directDeclarators;
    std::stack<std::unique_ptr<Declarator>> declarators;
    std::stack<std::unique_ptr<InitializedDeclarator>> initializedDeclarators;
    std::stack<std::vector<std::unique_ptr<InitializedDeclarator>>> initializedDeclaratorLists;
    std::stack<FormalArgument> formalArguments;
    std::stack<FormalArguments> formalArgumentLists;
    std::stack<std::pair<FormalArguments, bool>> argumentsDeclarations;
    std::stack<std::unique_ptr<Declaration>> declarations;
    std::stack<std::vector<std::unique_ptr<Declaration>>> declarationLists;
    std::stack<std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>> statementLists;
    std::stack<std::unique_ptr<AbstractSyntaxTreeNode>> externalDeclarations;
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit;
    std::stack<std::vector<std::pair<std::string, type::Type>>> structMemberLists;
    std::stack<std::vector<PendingMemberDeclarator>> pendingMemberDeclaratorLists;
    std::stack<std::vector<std::unique_ptr<Declarator>>> structDeclaratorLists;
    std::stack<bool> isUnionStack;

    struct PendingDesignator {
        std::vector<std::string> memberPath;
        std::optional<long> arrayIndex;
    };
    std::stack<PendingDesignator> pendingDesignators;
};

} // namespace ast

#endif /* ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_ */
