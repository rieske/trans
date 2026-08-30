#ifndef SEMANTICANALYSISVISITOR_H_
#define SEMANTICANALYSISVISITOR_H_

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "SymbolTable.h"
#include "symbols/AnnotationStore.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"
#include "ast/AbstractSyntaxTreeVisitor.h"

namespace ast {
class VlaExpressionTable;
}

namespace semantic_analyzer {

class SemanticAnalysisVisitor: public ast::AbstractSyntaxTreeVisitor {
public:
    virtual ~SemanticAnalysisVisitor() = default;

    void visit(ast::DeclarationSpecifiers& declarationSpecifiers) override;
    void visit(ast::Declaration& declaration) override;

    void visit(ast::Declarator& declarator) override;
    void visit(ast::InitializedDeclarator& declarator) override;

    void visit(ast::ArrayAccess& arrayAccess) override;
    void visit(ast::MemberAccess& memberAccess) override;
    void visit(ast::InitializerListExpression& expression) override;
    void visit(ast::FunctionCall& functionCall) override;
    void visit(ast::IdentifierExpression& identifier) override;
    void visit(ast::ConstantExpression& constant) override;
    void visit(ast::StringLiteralExpression& stringLiteral) override;
    void visit(ast::PostfixExpression& expression) override;
    void visit(ast::PrefixExpression& expression) override;
    void visit(ast::UnaryExpression& expression) override;
    void visit(ast::TypeCast& expression) override;
    void visit(ast::TypeNameExpression& expression) override;
    void visit(ast::CompoundLiteral& expression) override;
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
    void visit(ast::ExpressionList& expression) override;

    void visit(ast::JumpStatement& statement) override;
    void visit(ast::GotoStatement& statement) override;
    void visit(ast::LabeledStatement& statement) override;
    void visit(ast::SwitchStatement& statement) override;
    void visit(ast::CaseLabel& statement) override;
    void visit(ast::DefaultLabel& statement) override;
    void visit(ast::ReturnStatement& statement) override;
    void visit(ast::VoidReturnStatement& statement) override;
    void visit(ast::IfStatement& statement) override;
    void visit(ast::IfElseStatement& statement) override;
    void visit(ast::LoopStatement& statement) override;

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

    void setAnnotationStore(symbols::AnnotationStore& store) { store_ = &store; }
    void setGnuExtensions(bool enabled) { gnuExtensions_ = enabled; }
    void setVlaExpressions(ast::VlaExpressionTable* table) { vlas_ = table; }
    const ast::VlaExpressionTable& vlaTable() const;
    symbols::AnnotationStore& annotations() {
        if (!store_) {
            throw std::runtime_error { "AnnotationStore not set on SemanticAnalysisVisitor" };
        }
        return *store_;
    }

    // Import one parse-time enumerator into the symbol table (once per analyze).
    void importParseEnumConstant(const std::string& name, type::IntegerConstant value);
    void installGnuBuiltins();

    // Shared with initializer placement sinks (same package).
    // Assignment / init / call / return: dest <- source under productAssignOk.
    // Pass sourceExpr so foldable zero (e.g. ((void*)0)) is accepted into pointers.
    // Returns true on success; emits a diagnostic and returns false on failure.
    bool checkAssign(const type::Type& dest, const type::Type& source, const translation_unit::Context& context,
            const ast::Expression* sourceExpr = nullptr);
    // Binary / ternary operands: type-only product assign of left into right (legacy gate).
    bool checkOperandTypes(const type::Type& left, const type::Type& right,
            const translation_unit::Context& context);
    void semanticError(std::string message, const translation_unit::Context& context);
    // Insert-before-init for one declarator; specifiers supply resolved type and storage.
    void analyzeInitializedDeclarator(ast::InitializedDeclarator& declarator,
            const ast::DeclarationSpecifiers& specifiers);

private:
    bool rewriteCharArrayStringInitializer(ast::InitializedDeclarator& declarator, const type::Type& type);
    // Size incomplete arrays from a brace initializer; shared by declarators and compound literals.
    bool applyIncompleteArrayBound(type::Type& type, ast::Expression* init,
            const translation_unit::Context& context);
    bool completeArrayFromInitializer(ast::InitializedDeclarator& declarator, type::Type& type,
            bool& initializerVisited);
    // Local (non-.data) aggregate field plan on any annotated node.
    void planLocalAggregateFieldInits(symbols::NodeRef node, const type::Type& objectType,
            const ast::InitializerListExpression* list, const translation_unit::Context& context);
    // Scalar brace list policy for locals / compound literals (excess, unwrap, assign convert).
    void lowerLocalScalarBraceList(ast::InitializerListExpression& list, const type::Type& objectType,
            const translation_unit::Context& context);
    void lowerLocalInitializer(ast::InitializedDeclarator& declarator, const type::Type& objectType);
    void lowerStaticInit(const std::string& name, const type::Type& objectType, ast::Expression* init,
            const translation_unit::Context& context);
    void lowerStaticAggregateInit(const std::string& name, const type::Type& objectType,
            const ast::InitializerListExpression* list, const translation_unit::Context& context);
    void rejectFunctionValue(const type::Type& type, const translation_unit::Context& context);

    void checkObjectArrayBounds(ast::InitializedDeclarator& declarator, bool allowVla);

    // Innermost loop first: break → exit, continue → cont (entry for while, pre-increment for for).
    struct LoopContext {
        symbols::LabelEntry* entry;
        symbols::LabelEntry* cont;
        symbols::LabelEntry* exit;
    };
    std::vector<LoopContext> loopStack;
    std::vector<ast::SwitchStatement*> switchStack;

    // Named labels (goto targets) within the current function.
    std::map<std::string, symbols::LabelEntry> namedLabels;
    std::vector<ast::GotoStatement*> pendingGotos;

    bool containsSemanticErrors { false };

    // Return type of the function currently under analysis (for return checkAssign).
    std::optional<type::Type> currentReturnType;
    std::string currentFunctionName;

    SymbolTable symbolTable;
    symbols::AnnotationStore* store_ { nullptr };
    ast::VlaExpressionTable* vlas_ { nullptr };
    bool gnuExtensions_ { true };
};

} // namespace semantic_analyzer

#endif // SEMANTICANALYSISVISITOR_H_
