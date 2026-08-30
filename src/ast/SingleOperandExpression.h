#ifndef SINGLEOPERANDEXPRESSION_H_
#define SINGLEOPERANDEXPRESSION_H_

#include <memory>

#include "ast/Expression.h"

namespace ast {

class SingleOperandExpression: public Expression {
public:
    explicit SingleOperandExpression(std::unique_ptr<Expression> _operand);
    virtual ~SingleOperandExpression();

    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

    void visitOperand(AbstractSyntaxTreeVisitor& visitor);
    type::Type operandType() const;
    bool hasOperandSymbol(const symbols::AnnotationStore& store) const;
    symbols::ValueEntry* operandSymbol(symbols::AnnotationStore& store) const;
    symbols::ValueEntry* operandLvalueSymbol(symbols::AnnotationStore& store) const;
    Expression* getOperandExpression() const;

    bool isLval() const override;

    translation_unit::Context getContext() const override;

protected:
    const std::unique_ptr<Expression> _operand;
};

} // namespace ast

#endif // SINGLEOPERANDEXPRESSION_H_
