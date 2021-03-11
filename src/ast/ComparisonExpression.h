#ifndef _COMPARISON_EXPRESSION_H_
#define _COMPARISON_EXPRESSION_H_

#include <memory>

#include "semantic_analyzer/LabelEntry.h"
#include "DoubleOperandExpression.h"

namespace ast {

class ComparisonExpression: public DoubleOperandExpression {
public:
    ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    semantic_analyzer::LabelEntry* getFalsyLabel() const;
    void setFalsyLabel(semantic_analyzer::LabelEntry falsyLabel);
    semantic_analyzer::LabelEntry* getTruthyLabel() const;
    void setTruthyLabel(semantic_analyzer::LabelEntry truthyLabel);

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> truthyLabel { nullptr };
    std::unique_ptr<semantic_analyzer::LabelEntry> falsyLabel { nullptr };
};

} // namespace ast

#endif // _COMPARISON_EXPRESSION_H_
