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
#include "symbols/AnnotationStore.h"
#include "types/Type.h"
#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "symbols/FunctionEntry.h"

namespace builtins {
struct BuiltinDescriptor;
}

namespace semantic_analyzer {

using symbols::ValueEntry;
using symbols::LabelEntry;
using symbols::FunctionEntry;


class SemanticAnalysisVisitor: public ast::AbstractSyntaxTreeVisitor {
public:
    explicit SemanticAnalysisVisitor(symbols::AnnotationStore& store,
            ast::PendingArrayMemberStore& pendingArrayMembers);
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

    // Single late pass: re-fold ARRAY_SIZE bounds on struct members once file-scope
    // symbols exist. Mutates shared StructBody layout in place.
    void applyPendingArrayMemberBounds();

    // Analyze a function body after phase-1 registration and the late bound-fold pass.
    // visit(FunctionDefinition) only registers; the orchestrator calls this explicitly.
    void analyzeFunctionBody(ast::FunctionDefinition& function);

    // Import a parse-time enumerator into the symbol table (idempotent).
    void importParseEnumConstant(const std::string& name, long value);

private:
    // Visitor-internal diagnostics and product assign checks (AggregateInit uses AggregateInitHost).
    void semanticError(std::string message, const translation_unit::Context& context);
    void requireProductAssignable(const type::Type& dest, const type::Type& source,
            const translation_unit::Context& context);

    // Non-assign operand gates (TypeQuery product*Compatible).
    void requireValueCompatible(const type::Type& a, const type::Type& b,
            const translation_unit::Context& context);
    void requireArithmeticCompatible(const type::Type& a, const type::Type& b,
            const translation_unit::Context& context);

    // Visit expression then apply array-to-pointer decay (value context, C 6.3.2.1).
    // Prefer this over accept + scattered decayArrayInPlace at use sites.
    void analyzeAsRvalue(ast::Expression& expr);
    void analyzeAsRvalue(ast::Expression* expr);

    // Fold sizeof in ARRAY_SIZE-style bounds without semanticError on missing symbols.
    void foldSizeofInBound(ast::Expression* expr);

    // Register function in the symbol table (no body analysis).
    void registerFunctionDefinition(ast::FunctionDefinition& function);

    // FunctionCall helpers (Calls TU).
    struct ResolvedCallee {
        FunctionEntry symbol;
        bool indirect { false };
    };
    std::optional<ResolvedCallee> resolveCallee(const std::string& designatorName,
            symbols::ValueEntry* operandSym, const type::Type& operandType,
            const translation_unit::Context& callContext);
    void checkAndConvertCallArgs(ast::FunctionCall& functionCall, const FunctionEntry& functionSymbol);
    void analyzeBuiltinCall(ast::FunctionCall& functionCall, const std::string& builtinName,
            const builtins::BuiltinDescriptor& builtin);

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

    SymbolTable symbolTable;
    symbols::AnnotationStore& store_;
    ast::PendingArrayMemberStore& pendingArrayMembers_;
};

} // namespace semantic_analyzer

#endif // SEMANTICANALYSISVISITOR_H_
