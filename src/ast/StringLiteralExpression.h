#ifndef AST_STRING_LITERAL_EXPRESSION_H_
#define AST_STRING_LITERAL_EXPRESSION_H_

#include <string>

#include "translation_unit/Context.h"
#include "types/IntegralType.h"
#include "Expression.h"

namespace ast {

class StringLiteralExpression: public Expression {
public:
    StringLiteralExpression(std::string value, translation_unit::Context context);
    virtual ~StringLiteralExpression();

    translation_unit::Context getContext() const override;
    std::string getValue() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

private:
    std::string value;
    translation_unit::Context context;
};

} // namespace ast

#endif // AST_STRING_LITERAL_EXPRESSION_H_
