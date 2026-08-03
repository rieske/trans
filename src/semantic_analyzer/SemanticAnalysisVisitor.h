#ifndef SEMANTICANALYSISVISITOR_H_
#define SEMANTICANALYSISVISITOR_H_

#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "SymbolTable.h"
#include "ast/AbstractSyntaxTreeVisitor.h"

#include "ast/PendingArrayMemberStore.h"
#include "symbols/AnnotationStore.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"
#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "symbols/FunctionEntry.h"

namespace ast {
class AbstractSyntaxTree;
class LogicalExpression;
}

namespace builtins {
struct BuiltinDescriptor;
}

namespace semantic_analyzer {

class SemanticAnalysisVisitor: public ast::AbstractSyntaxTreeVisitor {
public:
    SemanticAnalysisVisitor() = default;
    virtual ~SemanticAnalysisVisitor() = default;

    void setAnnotationStore(symbols::AnnotationStore& store) { store_ = &store; }
    void setPendingArrayMembers(ast::PendingArrayMemberStore& pending) {
        pendingArrayMembers_ = &pending;
    }
    symbols::AnnotationStore& annotations() {
        if (!store_) {
            throw std::runtime_error { "AnnotationStore not set on SemanticAnalysisVisitor" };
        }
        return *store_;
    }
    ast::PendingArrayMemberStore& pendingArrayMembers() {
        if (!pendingArrayMembers_) {
            throw std::runtime_error { "PendingArrayMemberStore not set on SemanticAnalysisVisitor" };
        }
        return *pendingArrayMembers_;
    }

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
    void visit(ast::TypeNameExpression& expression) override;
    void visit(ast::TypeCast& expression) override;
    void visit(ast::GenericSelection& expression) override;
    void visit(ast::StatementExpression& expression) override;
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
    std::vector<symbols::ValueEntry> getDataHomes() const;

    // Import one parse-time enumerator into the symbol table (once per analyze).
    void importParseEnumConstant(const std::string& name, type::IntegerConstant value);
    void setGnuExtensions(bool enabled) { gnuExtensions_ = enabled; }
    void installGnuBuiltins();

private:
    // Re-fold ARRAY_SIZE bounds after a file-scope Declaration.
    void applyPendingArrayMemberBounds();

    // Visitor-internal diagnostics and product assign checks (AggregateInit uses AggregateInitHost).
    void semanticError(std::string message, const translation_unit::Context& context);
    // dest <- source via reportProductAssign. Pass sourceExpr so foldable zero
    // (e.g. ((void*)0)) is accepted into pointers. Returns false after diagnostic;
    // callers must not plan conversions on failure.
    bool checkProductAssign(const type::Type& dest, const type::Type& source,
            const translation_unit::Context& context, const ast::Expression* sourceExpr = nullptr);

    // Non-assign operand gates (TypeQuery product*Compatible). Returns false after diagnostic.
    bool checkValueCompatible(const type::Type& a, const type::Type& b,
            const translation_unit::Context& context);
    bool checkArithmeticCompatible(const type::Type& a, const type::Type& b,
            const translation_unit::Context& context);
    void checkIncrementOperand(bool isLval, const type::Type& operandType,
            const translation_unit::Context& context);

    // Visit expression then apply array-to-pointer decay (value context, C 6.3.2.1).
    // Prefer this over accept + scattered decayArrayInPlace at use sites.
    void analyzeAsRvalue(ast::Expression& expr);
    void analyzeAsRvalue(ast::Expression* expr);

    void analyzeLogical(ast::LogicalExpression& expression);

    // FunctionCall helpers (Calls TU).
    struct ResolvedCallee {
        symbols::FunctionEntry symbol;
        bool indirect { false };
    };
    std::optional<ResolvedCallee> resolveCallee(ast::Expression* operandExpr,
            symbols::ValueEntry* operandSym, const type::Type& operandType,
            const translation_unit::Context& callContext);
    void checkAndConvertCallArgs(ast::FunctionCall& functionCall, const symbols::FunctionEntry& functionSymbol);
    void analyzeBuiltinCall(ast::FunctionCall& functionCall, const std::string& builtinName,
            const builtins::BuiltinDescriptor& builtin);

    // Set while visiting a function body for implicit return conversions.
    std::optional<type::Type> currentFunctionReturnType;

    // Innermost loop/switch first: break target, continue target (null if none).
    struct LoopLabels {
        symbols::LabelEntry* breakLabel;
        symbols::LabelEntry* continueLabel;
    };
    std::vector<LoopLabels> loopStack;

    // Innermost switch for case/default registration.
    std::vector<ast::SwitchStatement*> switchStack;

    // Named labels (goto targets) within the current function.
    std::map<std::string, symbols::LabelEntry> namedLabels;
    std::vector<ast::GotoStatement*> pendingGotos;

    bool containsSemanticErrors { false };
    std::string currentFunctionName;

    SymbolTable symbolTable;
    symbols::AnnotationStore* store_ { nullptr };
    ast::PendingArrayMemberStore* pendingArrayMembers_ { nullptr };
    bool gnuExtensions_ { true };
};

} // namespace semantic_analyzer

#endif // SEMANTICANALYSISVISITOR_H_
