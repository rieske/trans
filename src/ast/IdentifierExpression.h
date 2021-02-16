#ifndef _IDENTIFIER_EXPRESSION_H_
#define _IDENTIFIER_EXPRESSION_H_

#include <string>

#include "Expression.h"

namespace ast {

class IdentifierExpression: public Expression {
public:
    IdentifierExpression(std::string identifier, translation_unit::Context context);
    virtual ~IdentifierExpression();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;
    std::string getIdentifier() const;

private:
    std::string identifier;
    translation_unit::Context context;
};

} // namespace ast

#endif // _IDENTIFIER_EXPRESSION_H_
