#ifndef ARRAYDECLARATION_H_
#define ARRAYDECLARATION_H_

#include <memory>
#include <vector>

#include "DirectDeclarator.h"
#include "Pointer.h"
#include "Expression.h"

namespace ast {

class ArrayDeclarator: public DirectDeclarator {
public:
    ArrayDeclarator(std::unique_ptr<DirectDeclarator> declarator, std::unique_ptr<Expression> subscriptExpression);
    virtual ~ArrayDeclarator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) override;

    void visitBaseDeclarator(AbstractSyntaxTreeVisitor& visitor);
    DirectDeclarator& getBaseDeclarator() const;

    void setArraySize(long size);
    long getArraySize() const;

    const std::unique_ptr<Expression> subscriptExpression;

private:
    std::unique_ptr<DirectDeclarator> baseDeclarator;
    long arraySize { 0 };
    bool arraySizeSet { false };
};

} // namespace ast

#endif // ARRAYDECLARATION_H_
