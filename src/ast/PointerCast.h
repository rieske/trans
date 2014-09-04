#ifndef _POINTER_CAST_NODE_H
#define _POINTER_CAST_NODE_H

#include <memory>

#include "SingleOperandExpression.h"
#include "TypeSpecifier.h"

namespace ast {

class Pointer;

class PointerCast: public SingleOperandExpression {
public:
    PointerCast(TypeSpecifier typeSpecifier, std::unique_ptr<Pointer> pointer, std::unique_ptr<Expression> castExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    TypeSpecifier getType() const;
    Pointer* getPointer() const;
private:
    const TypeSpecifier type;
    const std::unique_ptr<Pointer> pointer;
};

}

#endif // _POINTER_CAST_NODE_H
