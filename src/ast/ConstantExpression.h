#ifndef CONSTANTEXPRESSION_H_
#define CONSTANTEXPRESSION_H_

#include <string>

#include "Constant.h"
#include "Expression.h"

namespace ast {

class ConstantExpression: public Expression {
public:
    ConstantExpression(Constant constant);
    virtual ~ConstantExpression();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    translation_unit::Context getContext() const override;
    std::string getValue() const;

    bool evaluateConstant(type::IntegerConstant& value) const override;

private:
    Constant constant;
};

} // namespace ast

#endif // CONSTANTEXPRESSION_H_
