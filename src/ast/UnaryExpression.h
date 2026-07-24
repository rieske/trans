#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <memory>

#include "symbols/LabelEntry.h"
#include "ast/SingleOperandExpression.h"

namespace ast {

class UnaryExpression: public SingleOperandExpression {
public:
    UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    bool isLval() const override;
    bool evaluateConstant(long& value) const override;

    void setTruthyLabel(symbols::LabelEntry truthyLabel);
    symbols::LabelEntry* getTruthyLabel() const;
    void setFalsyLabel(symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getFalsyLabel() const;

    void setLvalueSymbol(symbols::ValueEntry lvalueSymbol);
    symbols::ValueEntry* getLvalueSymbol() const override;

    // sizeof / folded offsetof constant (bytes) after semantic analysis
    void setSizeofValue(int bytes);
    int getSizeofValue() const;
    bool isSizeof() const;

private:
    int sizeofValue { -1 };
};

} // namespace ast

#endif // _U_EXPR_NODE_H_
