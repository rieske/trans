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

private:
    std::unique_ptr<semantic_analyzer::ValueEntry> lvalue { nullptr };
};

} /* namespace ast */

#endif /* ARRAYACCESS_H_ */
