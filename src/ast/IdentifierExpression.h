#ifndef _IDENTIFIER_EXPRESSION_H_
#define _IDENTIFIER_EXPRESSION_H_

#include <optional>
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

    // Folded value for enumerators (and similar named constants).
    void setFoldedConstant(long value);
    void clearFoldedConstant();
    bool hasFoldedConstant() const;
    long getFoldedConstant() const;
    bool evaluateConstant(long& value) const override;

    // Function designator form/name: Expression::setFunctionDesignatorResult + FunctionDesignatorPlan.

    void setAsRvalue() { lval = false; }

private:
    std::string identifier;
    translation_unit::Context context;
    std::optional<long> foldedConstant;
};

} // namespace ast

#endif // _IDENTIFIER_EXPRESSION_H_
