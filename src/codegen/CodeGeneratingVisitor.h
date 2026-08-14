#ifndef CODEGENERATINGVISITOR_H_
#define CODEGENERATINGVISITOR_H_

#include <vector>

#include "Instruction.h"
#include "ast/AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
#include "types/Type.h"

namespace util {
struct FloatingBits;
}

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

    void visit(ast::Operator& op) override;

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
    void visit(ast::FunctionDeclarator& declaration) override;
    void visit(ast::ArrayDeclarator& declaration) override;

    void visit(ast::FormalArgument& parameter) override;

    void visit(ast::FunctionDefinition& function) override;

    void visit(ast::Block& block) override;

    IntermediateRepresentation takeIr();

private:
    void emit(Instruction instruction);
    bool tryEmitGnuDirectCall(ast::FunctionCall& functionCall, const std::string& calleeName);
    void emitBooleanConvert(const std::string& sourceName, const std::string& destName);
    void emitConvert(const std::string& sourceName, const std::string& destName,
            const type::Type& sourceType, const type::Type& destType);
    // Storage home: array object after call-arg decay (Lvalue), otherwise Result.
    symbols::ValueEntry* objectHome(ast::Expression& expression) const;
    // Result name after optional array decay (addressOf) or numeric/bool Conversion.
    std::string convertedResultName(ast::Expression& expression);
    void emitStructFieldInits(const std::string& objectName,
            const std::vector<symbols::StructFieldInit>& fieldStores);
    void emitAdditive(char op, const type::Type& leftType, const type::Type& rightType,
            const std::string& leftName, const std::string& rightName, const std::string& resultName);
    void emitMulDiv(char op, const std::string& left, const std::string& right,
            const std::string& result, const type::Type& resultType);
    void emitIntegerMulDiv(char op, const std::string& left, const std::string& right,
            const std::string& result, const type::Type& resultType);
    void emitComplexMulDiv(char op, const std::string& left, const std::string& right,
            const std::string& result, const type::Type& resultType);
    std::string addScratchValue(const type::Type& scratchType);
    void emitFloatingConstant(const std::string& dest, const util::FloatingBits& bits);
    void emitIncDec(const std::string& name, const type::Type& valueType, bool increment);
    void emitBitFieldExtract(const std::string& container, const std::string& dest,
            const type::BitField& bits);
    void emitBitFieldInsert(const std::string& addr, const std::string& value,
            const type::BitField& bits, const type::Type& unit);
    void emitLvalueStore(ast::Expression& lhs, const std::string& valueName);

    symbols::AnnotationStore& store_;
    IntermediateRepresentation module_;
    Procedure* currentProcedure_ { nullptr };
    std::vector<Instruction>* currentBody_ { nullptr };
    int convertLabel_ { 0 };
};

} // namespace codegen

#endif // CODEGENERATINGVISITOR_H_
