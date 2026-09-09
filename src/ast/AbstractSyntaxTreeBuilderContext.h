#ifndef ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_
#define ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_

#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "Constant.h"
#include "DeclarationSpecifiers.h"
#include "FormalArgument.h"
#include "FunctionDeclarator.h"
#include "ParseEnvironment.h"
#include "Pointer.h"
#include "StorageSpecifier.h"
#include "TerminalSymbol.h"
#include "TypeSpecifier.h"
#include "BlockItem.h"
#include "Declaration.h"
#include "ExternalDeclaration.h"
#include "InitializedDeclarator.h"
#include "InitializerListExpression.h"
#include "Declarator.h"
#include "GenericSelection.h"
#include "scanner/LexicalSession.h"

namespace diag {
class Sink;
}

namespace ast {

class Block;
class Statement;

class AbstractSyntaxTreeBuilderContext {
public:
    explicit AbstractSyntaxTreeBuilderContext(scanner::LexicalSession& session);
    AbstractSyntaxTreeBuilderContext(scanner::LexicalSession& session, ParseEnvironment& parent);
    virtual ~AbstractSyntaxTreeBuilderContext() = default;

    ParseEnvironment& environment() { return environment_; }

    void setSink(diag::Sink* sink);
    diag::Sink& sink() const;
    void error(const translation_unit::Context& where, std::string message);
    void fail() { failed_ = true; }
    bool failed() const { return failed_; }

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
    std::vector<Pointer> popPointers();

    void pushStatement(std::unique_ptr<Statement> statement);
    void pushStatement(std::unique_ptr<Expression> expression);
    void pushStatement(std::unique_ptr<Declaration> declaration);
    BlockItem popStatement();
    std::unique_ptr<Statement> popAsStatement();
    std::unique_ptr<Block> popBlock();

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

    void newStatementList(BlockItem item);
    void addToStatementList(BlockItem item);
    std::vector<BlockItem> popStatementList();

    void pushExternalDeclaration(ExternalDeclaration externalDeclaration);
    ExternalDeclaration popExternalDeclaration();

    void addToTranslationUnit(ExternalDeclaration externalDeclaration);
    std::vector<ExternalDeclaration> popTranslationUnit();

    // Struct definition support (member list frames + tag registry).
    void pushIsUnion(bool isUnion);
    bool popIsUnion();
    void newStructMemberList();
    void addStructMember(std::string name, type::Type memberType,
            std::optional<int> bitWidth = std::nullopt);
    void addStructEnumerators(std::vector<Enumerator> enumerators);
    struct RecordBody {
        std::vector<type::MemberSpec> members;
        std::vector<Enumerator> enumerators;
    };
    RecordBody popStructMemberList();
    // Scoped to the struct body, like the member list above: a struct defined inside a
    // member declaration must not drain the declarators the enclosing struct_decl holds.
    void newStructDeclaratorList();
    void addStructDeclarator(std::unique_ptr<Declarator> declarator,
            std::optional<int> bitWidth = std::nullopt);
    // Drains the current body's list; the frame stays for the next struct_decl.
    std::vector<std::pair<std::unique_ptr<Declarator>, std::optional<int>>> takeStructDeclarators();
    void popStructDeclaratorList();
    void pushGenericAssociation(GenericAssociation association);
    GenericAssociation popGenericAssociation();
    void newGenericAssocList(GenericAssociation association);
    void addGenericAssociation(GenericAssociation association);
    std::vector<GenericAssociation> popGenericAssocList();
    void newInitializerList();
    void addInitializerElement(InitializerElement element);
    std::vector<InitializerElement> popInitializerList();

    void pushMemberDesignator(std::string memberName);
    void pushArrayIndexDesignator(std::unique_ptr<Expression> indexExpression);
    // Push a fully-built designator (used when merging nested designator_list segments).
    void pushPendingDesignator(std::vector<DesignatorStep> steps);
    // Ordered designator steps (.a[1].b). Empty if none pending.
    void takePendingDesignator(std::vector<DesignatorStep>& steps);

private:
    std::stack<TerminalSymbol> terminalSymbols;

    std::stack<TypeSpecifier> typeSpecifiers;
    std::stack<StorageSpecifier> storageSpecifiers;
    std::stack<type::Qualifier> typeQualifiers;
    std::stack<std::vector<type::Qualifier>> typeQualifierLists;
    std::stack<Constant> constants;

    std::stack<DeclarationSpecifiers> declarationSpecifiersStack;

    std::stack<std::unique_ptr<Expression>> expressionStack;
    std::stack<std::vector<std::unique_ptr<Expression>>>actualArgumentLists;
    std::stack<std::vector<Pointer>> pointerStack;
    std::stack<BlockItem> statementStack;
    std::stack<std::unique_ptr<DirectDeclarator>> directDeclarators;
    std::stack<std::unique_ptr<Declarator>> declarators;
    std::stack<std::unique_ptr<InitializedDeclarator>> initializedDeclarators;
    std::stack<std::vector<std::unique_ptr<InitializedDeclarator>>>initializedDeclaratorLists;
    std::stack<FormalArgument> formalArguments;
    std::stack<FormalArguments> formalArgumentLists;
    std::stack<std::pair<FormalArguments, bool>> argumentsDeclarations;
    std::stack<std::unique_ptr<Declaration>> declarations;
    std::stack<std::vector<std::unique_ptr<Declaration>>> declarationLists;
    std::stack<std::vector<BlockItem>> statementLists;
    std::stack<ExternalDeclaration> externalDeclarations;
    std::vector<ExternalDeclaration> translationUnit;

    std::stack<bool> isUnionStack;
    std::stack<RecordBody> structMemberLists;
    std::stack<std::vector<std::pair<std::unique_ptr<Declarator>, std::optional<int>>>> structDeclaratorLists;
    std::stack<GenericAssociation> genericAssociations;
    std::stack<std::vector<GenericAssociation>> genericAssocLists;
    std::stack<std::vector<InitializerElement>> initializerLists;

    std::stack<std::vector<DesignatorStep>> pendingDesignators;

    ParseEnvironment environment_;
    diag::Sink* sink_ { nullptr };
    bool failed_ { false };
};

}
/* namespace ast */

#endif /* ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_ */
