#ifndef DOUBLEOPERANDEXPRESSION_H_
#define DOUBLEOPERANDEXPRESSION_H_

#include <memory>

#include "ast/Expression.h"
#include "ast/Operator.h"

namespace ast {

class DoubleOperandExpression: public Expression {
public:
    DoubleOperandExpression(std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand, std::unique_ptr<Operator> _operator);
    virtual ~DoubleOperandExpression();

    void visitLeftOperand(AbstractSyntaxTreeVisitor& visitor);
    void visitRightOperand(AbstractSyntaxTreeVisitor& visitor);

    Expression* getLeftOperand() const;
    Expression* getRightOperand() const;

    // C expression types of the operands (may be array before decay).
    type::Type leftOperandType() const;
    type::Type rightOperandType() const;
    // Result-symbol / value types (after array decay temps). Prefer for
    // arithmetic signedness and codegen value paths.
    type::Type leftValueType() const;
    type::Type rightValueType() const;

    bool hasLeftOperandSymbol() const;
    bool hasRightOperandSymbol() const;
    symbols::ValueEntry* leftOperandSymbol() const;
    symbols::ValueEntry* rightOperandSymbol() const;

    translation_unit::Context getContext() const override;

    Operator* getOperator() const;

    bool evaluateConstant(long& value) const override;

protected:
    const std::unique_ptr<Expression> leftOperand;
    const std::unique_ptr<Expression> rightOperand;

    const std::unique_ptr<Operator> _operator;
};

} // namespace ast

#endif // DOUBLEOPERANDEXPRESSION_H_
