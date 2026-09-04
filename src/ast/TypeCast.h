#ifndef TYPECAST_H_
#define TYPECAST_H_

#include <memory>

#include "ast/SingleOperandExpression.h"
#include "ast/TypeSpecifier.h"

namespace ast {

class TypeCast: public SingleOperandExpression {
public:
    TypeCast(TypeSpecifier typeSpecifier, std::unique_ptr<Expression> castExpression);
    virtual ~TypeCast();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::TypeCast; }
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

    // Target type specifier of the cast (not Expression::getType()).
    const TypeSpecifier& getTypeSpecifier() const;
    TypeSpecifier& getTypeSpecifier();
    // Casts are never lvalues in C (unlike the operand).
    bool isLval() const override;
    bool evaluateConstant(type::IntegerConstant& value) const override;

private:
    TypeSpecifier typeSpecifier;
};

} // namespace ast

#endif // TYPECAST_H_
