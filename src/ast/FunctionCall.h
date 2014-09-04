#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>

#include "SingleOperandExpression.h"

namespace ast {

class AssignmentExpressionList;

class FunctionCall: public SingleOperandExpression {
public:
    FunctionCall(std::unique_ptr<Expression> callExpression, std::unique_ptr<AssignmentExpressionList> argumentList);
    virtual ~FunctionCall();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    AssignmentExpressionList* getArgumentList() const;

private:
    const std::unique_ptr<AssignmentExpressionList> argumentList;
};

} /* namespace ast */

#endif /* FUNCTIONCALL_H_ */
