#ifndef TYPECAST_H_
#define TYPECAST_H_

#include <memory>

#include "ast/SingleOperandExpression.h"
#include "ast/TypeName.h"

namespace ast {

class TypeCast: public SingleOperandExpression {
public:
    TypeCast(TypeName typeName, std::unique_ptr<Expression> castExpression);
    virtual ~TypeCast();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    // Target type_name of the cast (not Expression::expressionType()).
    // Non-const so SA can resolve typeof / abstract declarator in place.
    TypeName& getTypeName();
    const TypeName& getTypeName() const;
    // Casts are never lvalues in C (unlike the operand).
    bool isLval() const override;
    bool evaluateConstant(long& value) const override;

private:
    TypeName typeName;
};

} // namespace ast

#endif // TYPECAST_H_
