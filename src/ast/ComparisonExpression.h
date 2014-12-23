#ifndef _ML_EXPR_NODE_H_
#define _ML_EXPR_NODE_H_

#include <memory>
#include <string>

#include "../code_generator/LabelEntry.h"
#include "DoubleOperandExpression.h"

namespace ast {

class ComparisonExpression: public DoubleOperandExpression {
public:
    ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    code_generator::LabelEntry* getFalsyLabel() const;
    void setFalsyLabel(code_generator::LabelEntry falsyLabel);
    code_generator::LabelEntry* getTruthyLabel() const;
    void setTruthyLabel(code_generator::LabelEntry truthyLabel);

    static const std::string DIFFERENCE;
    static const std::string EQUALITY;

private:
    std::unique_ptr<code_generator::LabelEntry> truthyLabel { nullptr };
    std::unique_ptr<code_generator::LabelEntry> falsyLabel { nullptr };
};

}

#endif // _ML_EXPR_NODE_H_
