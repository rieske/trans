#ifndef LOGICALEXPRESSION_H_
#define LOGICALEXPRESSION_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class LogicalExpression: public DoubleOperandExpression {
public:
    virtual ~LogicalExpression();

    void setExitLabel(SymbolEntry* exitLabel);
    SymbolEntry* getExitLabel() const;

protected:
    LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator, std::unique_ptr<Expression> rightHandSide);

private:
    SymbolEntry* exitLabel { nullptr };
};

} /* namespace ast */

#endif /* LOGICALEXPRESSION_H_ */
