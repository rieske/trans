#ifndef _ML_EXPR_NODE_H_
#define _ML_EXPR_NODE_H_

#include <memory>
#include <string>

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

    static const std::string RELATIONAL;
    static const std::string EQUALITY;

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> truthyLabel { nullptr };
    std::unique_ptr<semantic_analyzer::LabelEntry> falsyLabel { nullptr };
};

}

#endif // _ML_EXPR_NODE_H_
