#include "ArrayAccess.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "symbols/AnnotationStore.h"
namespace ast {
ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression)
    : DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression), std::unique_ptr<Operator>{new Operator("[]")}) {
    lval = true;
}
void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void ArrayAccess::setLvalue(symbols::ValueEntry lvalue) {
    symbols::AnnotationStore::current().setValue(this, symbols::ValueSlot::Lvalue, std::move(lvalue));
}
symbols::ValueEntry* ArrayAccess::getLvalue() const {
    return symbols::AnnotationStore::current().value(this, symbols::ValueSlot::Lvalue);
}
symbols::ValueEntry* ArrayAccess::getLvalueSymbol() const { return getLvalue(); }
void ArrayAccess::setElementSize(int sizeInBytes) { elementSize = sizeInBytes; }
int ArrayAccess::getElementSize() const { return elementSize; }
bool ArrayAccess::baseIsArray() const { return baseIsArrayFlag; }
void ArrayAccess::setBaseIsArray(bool value) { baseIsArrayFlag = value; }
} // namespace ast
