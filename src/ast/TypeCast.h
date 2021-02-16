#ifndef TYPECAST_H_
#define TYPECAST_H_

#include <memory>
#include <string>

#include "ast/SingleOperandExpression.h"
#include "ast/TypeSpecifier.h"

namespace ast {

class TypeCast: public SingleOperandExpression {
public:
    TypeCast(TypeSpecifier typeSpecifier, std::unique_ptr<Expression> castExpression);
    virtual ~TypeCast();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    TypeSpecifier getType() const;

    static const std::string ID;

private:
    const TypeSpecifier typeSpecifier;
};

} // namespace ast

#endif // TYPECAST_H_
