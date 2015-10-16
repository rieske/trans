#ifndef LOGICALEXPRESSION_H_
#define LOGICALEXPRESSION_H_

#include <memory>

#include "semantic_analyzer/LabelEntry.h"
#include "DoubleOperandExpression.h"

namespace ast {

class LogicalExpression: public DoubleOperandExpression {
public:
    virtual ~LogicalExpression();

    void setExitLabel(semantic_analyzer::LabelEntry exitLabel);
    semantic_analyzer::LabelEntry* getExitLabel() const;

protected:
    LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator, std::unique_ptr<Expression> rightHandSide);

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> exitLabel { nullptr };
};

} /* namespace ast */

#endif /* LOGICALEXPRESSION_H_ */
