#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>

#include "SingleOperandExpression.h"

namespace code_generator {
class FunctionEntry;
} /* namespace code_generator */

namespace ast {

class ArgumentExpressionList;

class FunctionCall: public SingleOperandExpression {
public:
    FunctionCall(std::unique_ptr<Expression> callExpression, std::unique_ptr<ArgumentExpressionList> argumentList);
    virtual ~FunctionCall();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    ArgumentExpressionList* getArgumentList() const;

    void setSymbol(code_generator::FunctionEntry symbol);
    code_generator::FunctionEntry* getSymbol() const;

private:
    const std::unique_ptr<ArgumentExpressionList> argumentList;

    std::unique_ptr<code_generator::FunctionEntry> symbol;
};

} /* namespace ast */

#endif /* FUNCTIONCALL_H_ */
