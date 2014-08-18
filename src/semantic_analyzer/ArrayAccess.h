#ifndef ARRAYACCESS_H_
#define ARRAYACCESS_H_

#include <memory>

#include "Expression.h"

namespace semantic_analyzer {

class ArrayAccess: public Expression {
public:
    ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression);
    virtual ~ArrayAccess();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLvalue(SymbolEntry* lvalue);
    SymbolEntry* getLvalue() const;

    const std::unique_ptr<Expression> postfixExpression;
    const std::unique_ptr<Expression> subscriptExpression;

private:
    SymbolEntry* lvalue { nullptr };
};

} /* namespace semantic_analyzer */

#endif /* ARRAYACCESS_H_ */
