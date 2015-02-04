#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>
#include <vector>

#include "SingleOperandExpression.h"
#include "code_generator/FunctionEntry.h"

namespace ast {

class FunctionCall: public SingleOperandExpression {
public:
    FunctionCall(std::unique_ptr<Expression> callExpression, std::vector<std::unique_ptr<Expression>> argumentList = { });
    virtual ~FunctionCall() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitArguments(AbstractSyntaxTreeVisitor& visitor);

    const std::vector<std::unique_ptr<Expression>>& getArgumentList() const;

    void setSymbol(code_generator::FunctionEntry symbol);
    code_generator::FunctionEntry* getSymbol() const;

private:
    std::vector<std::unique_ptr<Expression>> argumentList;
    std::unique_ptr<code_generator::FunctionEntry> symbol;
};

} /* namespace ast */

#endif /* FUNCTIONCALL_H_ */
