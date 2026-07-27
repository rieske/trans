#ifndef ARRAYACCESS_H_
#define ARRAYACCESS_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class ArrayAccess: public DoubleOperandExpression {
public:
    ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setElementSize(int sizeInBytes);
    int getElementSize() const;
    // Lvalue address temp: Expression::setLvalueSymbol / getLvalueSymbol (store).
    // Base LeaObject vs PointerValue is symbols::IndexPlan::baseMode on the store.

private:
    int elementSize { 0 };
};

} // namespace ast

#endif // ARRAYACCESS_H_
