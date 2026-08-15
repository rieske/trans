#ifndef AST_TYPENAMEEXPRESSION_H_
#define AST_TYPENAMEEXPRESSION_H_

#include "Expression.h"
#include "TypeSpecifier.h"
#include "translation_unit/Context.h"

namespace ast {

// A type-name used as an expression operand (sizeof(type-name)).
class TypeNameExpression: public Expression {
public:
    TypeNameExpression(TypeSpecifier typeSpecifier, translation_unit::Context context);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    translation_unit::Context getContext() const override;

    TypeSpecifier& typeSpecifier();
    const TypeSpecifier& typeSpecifier() const;

private:
    TypeSpecifier typeSpecifier_;
    translation_unit::Context context_;
};

} // namespace ast

#endif
