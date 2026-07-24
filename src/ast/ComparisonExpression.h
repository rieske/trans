#ifndef _COMPARISON_EXPRESSION_H_
#define _COMPARISON_EXPRESSION_H_

#include <memory>

#include "symbols/LabelEntry.h"
#include "DoubleOperandExpression.h"

namespace ast {

class ComparisonExpression: public DoubleOperandExpression {
public:
    ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    symbols::LabelEntry* getFalsyLabel() const;
    void setFalsyLabel(symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getTruthyLabel() const;
    void setTruthyLabel(symbols::LabelEntry truthyLabel);

private:
};

} // namespace ast

#endif // _COMPARISON_EXPRESSION_H_
