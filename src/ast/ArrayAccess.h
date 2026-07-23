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
    bool baseIsArray() const;
    void setBaseIsArray(bool value);

private:
    std::unique_ptr<semantic_analyzer::ValueEntry> lvalue { nullptr };
    int elementSize { 8 };
    bool baseIsArrayFlag { false };
};

} // namespace ast

#endif // ARRAYACCESS_H_
