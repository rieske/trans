#ifndef LOGICALEXPRESSION_H_
#define LOGICALEXPRESSION_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace code_generator {
class LabelEntry;
} /* namespace code_generator */

namespace ast {

class LogicalExpression: public DoubleOperandExpression {
public:
    virtual ~LogicalExpression();

    void setExitLabel(code_generator::LabelEntry* exitLabel);
    code_generator::LabelEntry* getExitLabel() const;

protected:
    LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator, std::unique_ptr<Expression> rightHandSide);

private:
    code_generator::LabelEntry* exitLabel { nullptr };
};

} /* namespace ast */

#endif /* LOGICALEXPRESSION_H_ */
