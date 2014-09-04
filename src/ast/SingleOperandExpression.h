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

    const TranslationUnitContext& getContext() const override;

    Expression* getOperand() const;
    Operator* getOperator() const;

protected:
    const std::unique_ptr<Expression> _operand;
    const std::unique_ptr<Operator> _operator;
};

} /* namespace ast */

#endif /* SINGLEOPERANDEXPRESSION_H_ */
