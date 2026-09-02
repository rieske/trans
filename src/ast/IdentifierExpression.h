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

    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    translation_unit::Context getContext() const override;
    std::string getIdentifier() const;

    // Parse-time const-fold residual for enumerators (not AnnotationStore).
    // SA may clear when an ordinary object shadows the name, or re-set from
    // the symbol table; codegen may read the final fold after SA.
    void setFoldedConstant(type::IntegerConstant value);
    // Drop a parse-time fold when SA binds this name to an object (shadow).
    void clearFoldedConstant();
    bool evaluateConstant(type::IntegerConstant& value) const override;

    void setRodataLabel(symbols::AnnotationStore& store, std::string label);
    const std::string* rodataLabel(const symbols::AnnotationStore& store) const;

private:
    std::string identifier;
    translation_unit::Context context;
    std::optional<type::IntegerConstant> foldedConstant;
};

} // namespace ast

#endif // _IDENTIFIER_EXPRESSION_H_
