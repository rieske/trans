#ifndef SINGLEOPERANDEXPRESSION_H_
#define SINGLEOPERANDEXPRESSION_H_

#include <memory>

#include "ast/Expression.h"
#include "ast/Operator.h"

namespace ast {

class SingleOperandExpression: public Expression {
public:
    SingleOperandExpression(std::unique_ptr<Expression> _operand, std::unique_ptr<Operator> _operator);
    virtual ~SingleOperandExpression();

    void visitOperand(AbstractSyntaxTreeVisitor& visitor);
    type::Type operandType() const;
    bool hasOperandSymbol() const;
    semantic_analyzer::ValueEntry* operandSymbol() const;
    semantic_analyzer::ValueEntry* operandLvalueSymbol() const;
    Expression* getOperandExpression() const;

    bool isLval() const override;

    translation_unit::Context getContext() const override;

    Operator* getOperator() const;

protected:
    const std::unique_ptr<Expression> _operand;
    const std::unique_ptr<Operator> _operator;
};

} // namespace ast

#endif // SINGLEOPERANDEXPRESSION_H_
