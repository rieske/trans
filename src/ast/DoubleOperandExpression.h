#ifndef DOUBLEOPERANDEXPRESSION_H_
#define DOUBLEOPERANDEXPRESSION_H_

#include <memory>
#include <string>

#include "ast/Expression.h"

namespace ast {

class DoubleOperandExpression: public Expression {
public:
    DoubleOperandExpression(std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand);
    virtual ~DoubleOperandExpression();

    std::optional<type::Type> intIfOperandsType(const ParseEnvironment& environment) const;

    void visitLeftOperand(AbstractSyntaxTreeVisitor& visitor);
    void visitRightOperand(AbstractSyntaxTreeVisitor& visitor);

    type::Type leftOperandType() const;
    type::Type rightOperandType() const;

    bool hasLeftOperandSymbol(const symbols::AnnotationStore& store) const;
    bool hasRightOperandSymbol(const symbols::AnnotationStore& store) const;
    symbols::ValueEntry* leftOperandSymbol(symbols::AnnotationStore& store) const;
    symbols::ValueEntry* rightOperandSymbol(symbols::AnnotationStore& store) const;
    Expression* getLeftOperand() const;
    Expression* getRightOperand() const;

    translation_unit::Context getContext() const override;

protected:
    bool foldOperands(type::IntegerConstant& value, const std::string& op) const;

    const std::unique_ptr<Expression> leftOperand;
    const std::unique_ptr<Expression> rightOperand;
};

} // namespace ast

#endif // DOUBLEOPERANDEXPRESSION_H_
