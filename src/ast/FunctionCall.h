#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>

#include "SingleOperandExpression.h"

namespace code_generator {
class FunctionEntry;
} /* namespace code_generator */

namespace ast {

class AssignmentExpressionList;

class FunctionCall: public SingleOperandExpression {
public:
    FunctionCall(std::unique_ptr<Expression> callExpression, std::unique_ptr<AssignmentExpressionList> argumentList);
    virtual ~FunctionCall();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    AssignmentExpressionList* getArgumentList() const;

    void setSymbol(code_generator::FunctionEntry symbol);
    code_generator::FunctionEntry* getSymbol() const;

private:
    const std::unique_ptr<AssignmentExpressionList> argumentList;

    std::unique_ptr<code_generator::FunctionEntry> symbol;
};

} /* namespace ast */

#endif /* FUNCTIONCALL_H_ */
