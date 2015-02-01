#ifndef SINGLEOPERANDEXPRESSION_H_
#define SINGLEOPERANDEXPRESSION_H_

#include <memory>

#include "Expression.h"

namespace ast {

class Operator;

class SingleOperandExpression: public Expression {
public:
    SingleOperandExpression(std::unique_ptr<Expression> _operand, std::unique_ptr<Operator> _operator);
    virtual ~SingleOperandExpression();

    void visitOperand(AbstractSyntaxTreeVisitor& visitor);
    Type operandType() const;
    code_generator::ValueEntry* operandSymbol() const;

    bool isLval() const override;

    translation_unit::Context getContext() const override;

    Operator* getOperator() const;

protected:
    const std::unique_ptr<Expression> _operand;
    const std::unique_ptr<Operator> _operator;
};

} /* namespace ast */

#endif /* SINGLEOPERANDEXPRESSION_H_ */
