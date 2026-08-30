#ifndef ARRAYDECLARATION_H_
#define ARRAYDECLARATION_H_

#include <memory>
#include <vector>

#include "DirectDeclarator.h"
#include "Pointer.h"
#include "Expression.h"

namespace ast {

class VlaExpressionTable;

enum class ArrayBoundFold { Complete, Unfixed, Negative, TooLarge };

class ArrayDeclarator: public DirectDeclarator {
public:
    ArrayDeclarator(std::unique_ptr<DirectDeclarator> declarator,
            std::unique_ptr<Expression> subscriptExpression, VlaExpressionTable* table = nullptr);
    virtual ~ArrayDeclarator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) const override;

    void visitBaseDeclarator(AbstractSyntaxTreeVisitor& visitor);
    void setArraySize(long size);
    bool hasArraySize() const;
    long getArraySize() const;

    void forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) override;
    const FunctionDeclarator* innermostFunctionDeclarator() const override;
    ArrayBoundFold foldOwnBound();

    const std::shared_ptr<Expression> subscriptExpression;

private:
    std::unique_ptr<DirectDeclarator> baseDeclarator;
    std::shared_ptr<type::VlaBound> vlaBound_;
    long arraySize { 0 };
    bool arraySizeSet { false };
};

} // namespace ast

#endif // ARRAYDECLARATION_H_
