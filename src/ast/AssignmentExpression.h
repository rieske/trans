#ifndef _A_EXPR_NODE_H_
#define _A_EXPR_NODE_H_

#include <memory>
#include <string>

#include "BinaryOpExpression.h"

namespace ast {

class AssignmentExpression: public BinaryOpExpression {
public:
    AssignmentExpression(std::unique_ptr<Expression> leftHandSide, std::string lexeme,
            std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

    // C: assignment expression is never an lvalue. SA checks leftOperand->isLval().
    bool isLval() const override;
    bool evaluateConstant(type::IntegerConstant&) const override { return false; }

    symbols::ValueEntry* leftOperandLvalueSymbol(symbols::AnnotationStore& store) const;
};

} // namespace ast

#endif // _A_EXPR_NODE_H_
