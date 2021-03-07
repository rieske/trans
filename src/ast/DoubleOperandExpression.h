#ifndef DOUBLEOPERANDEXPRESSION_H_
#define DOUBLEOPERANDEXPRESSION_H_

#include <memory>

#include "ast/Expression.h"
#include "ast/Operator.h"

namespace ast {

class DoubleOperandExpression: public Expression {
public:
    DoubleOperandExpression(std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand, std::unique_ptr<Operator> _operator);
    virtual ~DoubleOperandExpression();

    void visitLeftOperand(AbstractSyntaxTreeVisitor& visitor);
    void visitRightOperand(AbstractSyntaxTreeVisitor& visitor);

    type::Type leftOperandType() const;
    type::Type rightOperandType() const;

    semantic_analyzer::ValueEntry* leftOperandSymbol() const;
    semantic_analyzer::ValueEntry* rightOperandSymbol() const;

    translation_unit::Context getContext() const override;

    Operator* getOperator() const;

protected:
    const std::unique_ptr<Expression> leftOperand;
    const std::unique_ptr<Expression> rightOperand;

    const std::unique_ptr<Operator> _operator;
};

} // namespace ast

#endif // DOUBLEOPERANDEXPRESSION_H_
