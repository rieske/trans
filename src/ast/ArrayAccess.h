#ifndef ARRAYACCESS_H_
#define ARRAYACCESS_H_


#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class ArrayAccess: public DoubleOperandExpression {
public:
    ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    // Index stride / AddressBaseResolved live in symbols::IndexPlan on the store.
};

} // namespace ast

#endif // ARRAYACCESS_H_
