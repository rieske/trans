#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <memory>

#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"
#include "ast/SingleOperandExpression.h"

namespace ast {

class UnaryExpression: public SingleOperandExpression {
public:
    UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    bool isLval() const override;
    bool evaluateConstant(long& value) const override;

    void setTruthyLabel(symbols::AnnotationStore& store, symbols::LabelEntry truthyLabel);
    symbols::LabelEntry* getTruthyLabel(symbols::AnnotationStore& store) const;
    void setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getFalsyLabel(symbols::AnnotationStore& store) const;

    void setSizeofValue(int bytes);
    int getSizeofValue() const;

private:
    // Residual SA product for sizeof fold; feeds evaluateConstant. Optional later store.
    int sizeofValue { -1 };
};

} // namespace ast

#endif // _U_EXPR_NODE_H_
