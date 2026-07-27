#ifndef ARRAYACCESS_H_
#define ARRAYACCESS_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class ArrayAccess: public DoubleOperandExpression {
public:
    ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    // Lvalue address temp lives on AnnotationStore (ValueSlot::Lvalue).
    void setLvalue(symbols::AnnotationStore& store, symbols::ValueEntry lvalue);
    symbols::ValueEntry* getLvalue(symbols::AnnotationStore& store) const;
    symbols::ValueEntry* getLvalueSymbol(symbols::AnnotationStore& store) const override;

    void setElementSize(int sizeInBytes);
    int getElementSize() const;
    // Base LeaObject vs PointerValue is symbols::IndexPlan::baseMode on the store.

private:
    int elementSize { 0 };
};

} // namespace ast

#endif // ARRAYACCESS_H_
