#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <memory>
#include <string>

#include "SingleOperandExpression.h"

namespace code_generator {
class LabelEntry;
} /* namespace code_generator */

namespace ast {

class UnaryExpression: public SingleOperandExpression {
public:
    UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setTruthyLabel(code_generator::LabelEntry* truthyLabel);
    code_generator::LabelEntry* getTruthyLabel() const;
    void setFalsyLabel(code_generator::LabelEntry* falsyLabel);
    code_generator::LabelEntry* getFalsyLabel() const;

    static const std::string ID;

private:
    code_generator::LabelEntry* truthyLabel { nullptr };
    code_generator::LabelEntry* falsyLabel { nullptr };
};

}

#endif // _U_EXPR_NODE_H_
