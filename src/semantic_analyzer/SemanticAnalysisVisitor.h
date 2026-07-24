#ifndef SEMANTICANALYSISVISITOR_H_
#define SEMANTICANALYSISVISITOR_H_

#include <map>
#include <string>
#include <vector>

#include "SymbolTable.h"
#include "ast/AbstractSyntaxTreeVisitor.h"
#include <map>
#include <optional>

#include "ast/PendingArrayMemberStore.h"
#include "Symbols.h"

namespace semantic_analyzer {


class SemanticAnalysisVisitor: public ast::AbstractSyntaxTreeVisitor {
public:
    SemanticAnalysisVisitor();
    virtual ~SemanticAnalysisVisitor();

    void visit(ast::DeclarationSpecifiers& declarationSpecifiers) override;
    void visit(ast::Declaration& declaration) override;

    void visit(ast::Declarator& declarator) override;
    void visit(ast::InitializedDeclarator& declarator) override;

    void visit(ast::ArrayAccess& arrayAccess) override;
    void visit(ast::FunctionCall& functionCall) override;
    void visit(ast::IdentifierExpression& identifier) override;
    void visit(ast::ConstantExpression& constant) override;
    void visit(ast::StringLiteralExpression& stringLiteral) override;
    void visit(ast::PostfixExpression& expression) override;
    void visit(ast::PrefixExpression& expression) override;
    void visit(ast::UnaryExpression& expression) override;
    void visit(ast::TypeCast& expression) override;
    void visit(ast::ArithmeticExpression& expression) override;
    void visit(ast::ShiftExpression& expression) override;
    void visit(ast::ComparisonExpression& expression) override;
    void visit(ast::BitwiseExpression& expression) override;
    void visit(ast::LogicalAndExpression& expression) override;
    void visit(ast::LogicalOrExpression& expression) override;
    void visit(ast::ConditionalExpression& expression) override;
    void visit(ast::AssignmentExpression& expression) override;
    void visit(ast::MemberAccess& expression) override;
    void visit(ast::InitializerListExpression& expression) override;
    void visit(ast::CompoundLiteralExpression& expression) override;
    void visit(ast::ExpressionList& expression) override;

    void visit(ast::Operator& op) override;

    void visit(ast::JumpStatement& statement) override;
    void visit(ast::ReturnStatement& statement) override;
    void visit(ast::VoidReturnStatement& statement) override;
    void visit(ast::IfStatement& statement) override;
    void visit(ast::IfElseStatement& statement) override;
    void visit(ast::LoopStatement& statement) override;
    void visit(ast::SwitchStatement& statement) override;
    void visit(ast::CaseLabel& statement) override;
    void visit(ast::DefaultLabel& statement) override;
    void visit(ast::GotoStatement& statement) override;
    void visit(ast::LabeledStatement& statement) override;

    void visit(ast::ForLoopHeader& loopHeader) override;
    void visit(ast::WhileLoopHeader& loopHeader) override;
    void visit(ast::DoWhileLoopHeader& loopHeader) override;

    void visit(ast::Pointer& pointer) override;

    void visit(ast::Identifier& identifier) override;
    void visit(ast::FunctionDeclarator& declarator) override;
    void visit(ast::ArrayDeclarator& declaration) override;

    void visit(ast::FormalArgument& parameter) override;

    void visit(ast::FunctionDefinition& function) override;

    void visit(ast::Block& block) override;

    bool successfulSemanticAnalysis() const;
    std::map<std::string, std::string> getConstants() const;
    std::vector<ValueEntry> getGlobalVariables() const;

    void printSymbolTable() const;

    // Bound for this translation unit (from AbstractSyntaxTree); not process-global.
    void setPendingArrayMembers(ast::PendingArrayMemberStore* store) { pendingArrayMembers = store; }

    // Phase 1: register function symbols / formals only (skip bodies).
    void setSkipFunctionBodies(bool skip) { skipFunctionBodies = skip; }

    // Single late pass: re-fold ARRAY_SIZE bounds on struct members once file-scope
    // symbols exist. Mutates shared StructBody layout in place.
    void applyPendingArrayMemberBounds();

    // Phase 2: analyze a function body after the late bound-fold pass.
    void analyzeFunctionBody(ast::FunctionDefinition& function);

private:
    // Soft product assign gate (type::productCanAssignFrom) — not ISO can_assign.
    void requireProductAssignable(const type::Type& source, const type::Type& dest,
            const translation_unit::Context& context);
    void semanticError(std::string message, const translation_unit::Context& context);

    // Visit expression then apply array-to-pointer decay (value context, C 6.3.2.1).
    // Prefer this over accept + scattered decayArrayInPlace at use sites.
    void analyzeAsRvalue(ast::Expression& expr);
    void analyzeAsRvalue(ast::Expression* expr);

    // Fold sizeof in ARRAY_SIZE-style bounds without semanticError on missing symbols.
    void foldSizeofInBound(ast::Expression* expr);

    // Register function in the symbol table; when skipFunctionBodies, do not enter body.
    void registerFunctionDefinition(ast::FunctionDefinition& function);

    std::vector<std::string> argumentNames;
    // Set while visiting a function body for implicit return conversions.
    std::optional<type::Type> currentFunctionReturnType;

    // Innermost loop/switch first: break target, continue target (null if none).
    struct LoopLabels {
        LabelEntry* breakLabel;
        LabelEntry* continueLabel;
    };
    std::vector<LoopLabels> loopStack;

    // Innermost switch for case/default registration.
    std::vector<ast::SwitchStatement*> switchStack;

    // Named labels (goto targets) within the current function.
    std::map<std::string, LabelEntry> namedLabels;
    std::vector<ast::GotoStatement*> pendingGotos;

    bool containsSemanticErrors { false };
    bool skipFunctionBodies { false };

    SymbolTable symbolTable;
    ast::PendingArrayMemberStore* pendingArrayMembers { nullptr };
};

} // namespace semantic_analyzer

#endif // SEMANTICANALYSISVISITOR_H_
