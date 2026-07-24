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
    bool hasFoldedConstant() const;
    long getFoldedConstant() const;
    bool evaluateConstant(long& value) const override;

    // Function designator used as a value (decays to pointer-to-function).
    void setFunctionDesignator(std::string name);
    bool hasFunctionDesignator() const;
    const std::string& getFunctionDesignator() const;

private:
    std::string identifier;
    translation_unit::Context context;
    std::optional<long> foldedConstant;
    std::optional<std::string> functionDesignator;
};

} // namespace ast

#endif // _IDENTIFIER_EXPRESSION_H_
