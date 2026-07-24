#ifndef LOGICALEXPRESSION_H_
#define LOGICALEXPRESSION_H_

#include <memory>

#include "symbols/LabelEntry.h"
#include "ast/DoubleOperandExpression.h"

namespace ast {

class LogicalExpression: public DoubleOperandExpression {
public:
    virtual ~LogicalExpression();

    void setExitLabel(symbols::LabelEntry exitLabel);
    symbols::LabelEntry* getExitLabel() const;

protected:
    LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator, std::unique_ptr<Expression> rightHandSide);

private:
};

} // namespace ast

#endif // LOGICALEXPRESSION_H_
