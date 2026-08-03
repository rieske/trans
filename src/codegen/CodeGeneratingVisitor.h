#ifndef CODEGENERATINGVISITOR_H_
#define CODEGENERATINGVISITOR_H_

#include <map>
#include <string>
#include <vector>

#include "Instruction.h"
#include "ast/AbstractSyntaxTreeVisitor.h"
#include "types/Type.h"
#include "symbols/AnnotationStore.h"
#include "symbols/ValueEntry.h"

namespace codegen {

class CodeGeneratingVisitor: public ast::AbstractSyntaxTreeVisitor {
public:
    explicit CodeGeneratingVisitor(symbols::AnnotationStore& store);
    virtual ~CodeGeneratingVisitor();

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
    void visit(ast::FunctionDeclarator& declaration) override;
    void visit(ast::ArrayDeclarator& declaration) override;

    void visit(ast::FormalArgument& parameter) override;

    void visit(ast::FunctionDefinition& function) override;

    void visit(ast::Block& block) override;

    // Seal, peephole, then pack frames. Production and tests use this only.
    IntermediateRepresentation takeFinishedIr();

    void emit(Instruction instruction);

    // Visit expression, materialize array decay and optional conversion; return value symbol.
    std::string generateExpression(ast::Expression& expression);
    // Emit address of operand into dest without loading (for unary & and similar).
    void emitAddressOf(ast::Expression& operand, const std::string& destName);

private:
    void packFrames(IntermediateRepresentation& ir);
    // After 32/16/8-bit arithmetic, re-extend so high bits do not pollute shifts.
    void narrowIntegralResult(const type::Type& resultType, const std::string& resultName);

    IntermediateRepresentation module_;
    std::vector<Instruction> instructions;
    std::map<std::string, std::map<std::string, symbols::ValueEntry>> localsByProcedure_;
    symbols::AnnotationStore& store_;
};

} // namespace codegen

#endif // CODEGENERATINGVISITOR_H_
