#ifndef LOGICALEXPRESSION_H_
#define LOGICALEXPRESSION_H_

#include <memory>

#include "../code_generator/LabelEntry.h"
#include "DoubleOperandExpression.h"

namespace ast {

class LogicalExpression: public DoubleOperandExpression {
public:
    virtual ~LogicalExpression();

    void setExitLabel(code_generator::LabelEntry exitLabel);
    code_generator::LabelEntry* getExitLabel() const;

protected:
    LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator, std::unique_ptr<Expression> rightHandSide);

private:
    std::unique_ptr<code_generator::LabelEntry> exitLabel { nullptr };
};

} /* namespace ast */

#endif /* LOGICALEXPRESSION_H_ */
