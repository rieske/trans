#ifndef ARRAYDECLARATION_H_
#define ARRAYDECLARATION_H_

#include <functional>
#include <memory>
#include <vector>

#include "DirectDeclarator.h"
#include "Pointer.h"
#include "Expression.h"

namespace ast {

enum class ArrayBoundFold { Complete, Unfixed, Negative, TooLarge };

class ArrayDeclarator: public DirectDeclarator {
public:
    ArrayDeclarator(std::unique_ptr<DirectDeclarator> declarator, std::unique_ptr<Expression> subscriptExpression);
    virtual ~ArrayDeclarator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) const override;

    void visitBaseDeclarator(AbstractSyntaxTreeVisitor& visitor);
    DirectDeclarator& getBaseDeclarator() const;

    void setArraySize(long size);
    bool hasArraySize() const;
    long getArraySize() const;

    void forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) override;
    ArrayBoundFold foldOwnBound();

    void foldArrayBoundSizeofs(const std::function<void(Expression*)>& foldSizeof) override;
    bool hasArrayDeclarator() const override { return true; }

    const std::unique_ptr<Expression> subscriptExpression;

private:
    std::unique_ptr<DirectDeclarator> baseDeclarator;
    long arraySize { 0 };
    bool arraySizeSet { false };
};

} // namespace ast

#endif // ARRAYDECLARATION_H_
