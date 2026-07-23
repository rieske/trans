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

    Expression* getLeftOperand() const;
    Expression* getRightOperand() const;

    type::Type leftOperandType() const;
    type::Type rightOperandType() const;

    bool hasLeftOperandSymbol() const;
    bool hasRightOperandSymbol() const;
    semantic_analyzer::ValueEntry* leftOperandSymbol() const;
    semantic_analyzer::ValueEntry* rightOperandSymbol() const;

    translation_unit::Context getContext() const override;

    Operator* getOperator() const;

    bool evaluateConstant(long& value) const override;

protected:
    const std::unique_ptr<Expression> leftOperand;
    const std::unique_ptr<Expression> rightOperand;

    const std::unique_ptr<Operator> _operator;
};

} // namespace ast

#endif // DOUBLEOPERANDEXPRESSION_H_
