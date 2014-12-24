#ifndef ARRAYACCESS_H_
#define ARRAYACCESS_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class ArrayAccess: public DoubleOperandExpression {
public:
    ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression);
    virtual ~ArrayAccess();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLvalue(code_generator::ValueEntry lvalue);
    code_generator::ValueEntry* getLvalue() const;

private:
    std::unique_ptr<code_generator::ValueEntry> lvalue { nullptr };
};

} /* namespace ast */

#endif /* ARRAYACCESS_H_ */
