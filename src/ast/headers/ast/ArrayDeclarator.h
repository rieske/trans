#ifndef ARRAYDECLARATION_H_
#define ARRAYDECLARATION_H_

#include <memory>
#include <vector>

#include "DirectDeclarator.h"
#include "Pointer.h"

namespace ast {
class StoredType;
} /* namespace ast */

namespace ast {

class Expression;

class ArrayDeclarator: public DirectDeclarator {
public:
    ArrayDeclarator(std::unique_ptr<DirectDeclarator> declarator, std::unique_ptr<Expression> subscriptExpression);
    virtual ~ArrayDeclarator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    std::unique_ptr<FundamentalType> getFundamentalType(std::vector<Pointer> indirection, const FundamentalType& baseType) override;

    const std::unique_ptr<Expression> subscriptExpression;
};

} /* namespace ast */

#endif /* ARRAYDECLARATION_H_ */
