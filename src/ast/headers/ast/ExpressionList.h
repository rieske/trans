#ifndef EXPRESSIONLIST_H_
#define EXPRESSIONLIST_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class ExpressionList: public DoubleOperandExpression {
public:
    ExpressionList(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide);
    virtual ~ExpressionList();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} /* namespace ast */

#endif /* EXPRESSIONLIST_H_ */
