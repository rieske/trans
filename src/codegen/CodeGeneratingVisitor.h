#ifndef CODEGENERATINGVISITOR_H_
#define CODEGENERATINGVISITOR_H_

#include <map>
#include <string>
#include <vector>

#include "Instruction.h"
#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/Operator.h"
#include "types/Type.h"
#include "symbols/AnnotationStore.h"
#include "symbols/ValueEntry.h"

namespace type {
struct IntegerConstant;
}

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
    void visit(ast::FunctionDeclarator& declaration) override;
    void visit(ast::ArrayDeclarator& declaration) override;

    void visit(ast::FormalArgument& parameter) override;

    void visit(ast::FunctionDefinition& function) override;

    void visit(ast::Block& block) override;

    // Seal, peephole, then pack frames. Production and tests use this only.
    IntermediateRepresentation takeFinishedIr();

    void emit(Instruction instruction);

    // Visit expression, then field/index load, array decay, and conversion; return value symbol.
    std::string generateExpression(ast::Expression& expression);
    // Emit address of operand into dest without loading (for unary & and similar).
    void emitAddressOf(ast::Expression& operand, const std::string& destName);
    // Apply SA ValueSlot::Conversion after the expression result exists.
    std::string materializeConversion(ast::Expression& expression);
    // Procedure-local scratch (frame-registered); used by multi-TU helpers.
    std::string addScratchValue(const type::Type& scratchType);
    // Address of an array object: VLA home already holds it, else LEA the frame object.
    void emitArrayObjectAddress(const symbols::ValueEntry& object, const std::string& dest);
    void emitArrayObjectAddress(const std::string& objectName, const std::string& dest);
    void emitSizeofProduct(const type::Type& measured, const std::string& result);
    struct ScaledIndex {
        std::string name;
        int strideBytes;
    };
    ScaledIndex scaleIndex(const type::Type& objectType,
            const std::string& indexName, int constantStrideBytes);
    void emitIntegerConstant(const type::IntegerConstant& value, const std::string& dest);
    void emitBitFieldExtract(const std::string& container, const std::string& dest,
            const type::BitField& bits);
    void emitBitFieldInsert(const std::string& addr, const std::string& value,
            const type::BitField& bits, const type::Type& unit);

private:
    void packFrames(IntermediateRepresentation& ir);
    // After 32/16/8-bit arithmetic, re-extend so high bits do not pollute shifts.
    void narrowIntegralResult(const type::Type& resultType, const std::string& resultName);
    void emitBooleanConvert(const std::string& sourceName, const std::string& destName);
    void emitConvert(const std::string& sourceName, const std::string& destName,
            const type::Type& sourceType, const type::Type& destType);
    void emitMulDiv(ast::OperatorKind kind, const std::string& left, const std::string& right,
            const std::string& result, const type::Type& resultType, bool unsignedDiv = false);
    void emitComplexMulDiv(ast::OperatorKind kind, const std::string& left, const std::string& right,
            const std::string& result, const type::Type& resultType);
    void emitFloatingConstant(const std::string& dest, const util::FloatingBits& bits);
    void emitIncDec(const std::string& name, const type::Type& valueType, bool increment);
    void emitLvalueStore(ast::Expression& lhs, const std::string& valueName);

    IntermediateRepresentation module_;
    std::vector<Instruction> instructions;
    std::map<std::string, std::map<std::string, symbols::ValueEntry>> localsByProcedure_;
    std::map<std::string, symbols::ValueEntry>* currentLocals_ { nullptr };
    symbols::AnnotationStore& store_;
    int convertLabel_ { 0 };
};

} // namespace codegen

#endif // CODEGENERATINGVISITOR_H_
