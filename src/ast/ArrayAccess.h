#ifndef ARRAYACCESS_H_
#define ARRAYACCESS_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class ArrayAccess: public DoubleOperandExpression {
public:
    ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLvalue(semantic_analyzer::ValueEntry lvalue);
    semantic_analyzer::ValueEntry* getLvalue() const;
    semantic_analyzer::ValueEntry* getLvalueSymbol() const override;

    void setElementSize(int sizeInBytes);
    int getElementSize() const;
    void setBaseIsArray(bool value);
    bool baseIsArray() const;
    // When the selected element is itself an array (e.g. a[i] for int a[n][m]),
    // the access yields an address rather than a loaded scalar.
    void setYieldsAddress(bool value);
    bool yieldsAddress() const;

private:
    std::unique_ptr<semantic_analyzer::ValueEntry> lvalue { nullptr };
    int elementSize { 0 };
    bool baseIsArrayFlag { false };
    bool yieldsAddressFlag { false };
};

} // namespace ast

#endif // ARRAYACCESS_H_
