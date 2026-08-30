#ifndef CODEGENERATINGVISITOR_H_
#define CODEGENERATINGVISITOR_H_

#include <string>
#include <string_view>
#include <vector>

#include "Instruction.h"
#include "ast/AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"
#include "types/Type.h"

namespace ast {
class VlaExpressionTable;
}

namespace type {
struct IntegerConstant;
}

namespace util {
struct FloatingBits;
}

namespace codegen {

class CodeGeneratingVisitor: public ast::AbstractSyntaxTreeVisitor {
public:
    explicit CodeGeneratingVisitor(symbols::AnnotationStore& store,
            const ast::VlaExpressionTable* vlas = nullptr);
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
    void visit(ast::FunctionDeclarator& declaration) override;
    void visit(ast::ArrayDeclarator& declaration) override;

    void visit(ast::FormalArgument& parameter) override;

    void visit(ast::FunctionDefinition& function) override;

    void visit(ast::Block& block) override;

    IntermediateRepresentation takeIr();

private:
    void emit(Instruction instruction);
    int id(std::string_view name);
    int id(const symbols::ValueEntry& symbol);
    int id(const symbols::LabelEntry& label);
    bool tryEmitGnuDirectCall(ast::FunctionCall& functionCall, const std::string& calleeName);
    void emitBooleanConvert(int source, int dest);
    void emitConvert(int source, int dest,
            const type::Type& sourceType, const type::Type& destType);
    // Storage home: array object after call-arg decay (Lvalue), otherwise Result.
    symbols::ValueEntry* objectHome(ast::Expression& expression) const;
    // Address of an array object: VLA home already holds it, else LEA the frame object.
    void emitArrayObjectAddress(const symbols::ValueEntry& object, int dest);
    // Result after optional array decay (addressOf) or numeric/bool Conversion.
    int convertedResult(ast::Expression& expression);
    void emitStructFieldInits(int object,
            const std::vector<symbols::StructFieldInit>& fieldStores);
    void emitAdditive(char op, const type::Type& leftType, const type::Type& rightType,
            int left, int right, int result);
    void emitMulDiv(char op, int left, int right, int result, const type::Type& resultType);
    void emitIntegerMulDiv(char op, int left, int right, int result, const type::Type& resultType);
    void emitComplexMulDiv(char op, int left, int right, int result, const type::Type& resultType);
    int addScratchValue(const type::Type& scratchType);
    void emitSizeofProduct(const type::Type& measured, int result);
    struct ScaledIndex {
        int name;
        int strideBytes;
    };
    ScaledIndex scaleIndex(const type::Type& objectType,
            int indexName, int constantStrideBytes);
    void emitFloatingConstant(int dest, const util::FloatingBits& bits);
    void emitIntegerConstant(const type::IntegerConstant& value, int dest);
    void emitIncDec(int name, const type::Type& valueType, bool increment);
    void emitBitFieldExtract(int container, int dest, const type::BitField& bits);
    void emitBitFieldInsert(int addr, int value,
            const type::BitField& bits, const type::Type& unit);
    void emitLvalueStore(ast::Expression& lhs, int value);

    symbols::AnnotationStore& store_;
    const ast::VlaExpressionTable* vlas_ { nullptr };
    IntermediateRepresentation module_;
    Procedure* currentProcedure_ { nullptr };
    std::vector<Instruction>* currentBody_ { nullptr };
    int convertLabel_ { 0 };
};

} // namespace codegen

#endif // CODEGENERATINGVISITOR_H_
