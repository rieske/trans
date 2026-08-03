#ifndef AST_TYPENAMEEXPRESSION_H_
#define AST_TYPENAMEEXPRESSION_H_

#include "Expression.h"
#include "TypeName.h"
#include "translation_unit/Context.h"

namespace ast {

// type_name used as a sizeof operand: sizeof(T) / sizeof(T[N]) / sizeof(__typeof__(*p)).
// TypeName holds spec + abstract declarator (VLA / pointer / function).
class TypeNameExpression: public Expression {
public:
    TypeNameExpression(TypeSpecifier typeSpecifier, translation_unit::Context context);
    TypeNameExpression(TypeName typeName, translation_unit::Context context);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    translation_unit::Context getContext() const override;

    TypeSpecifier& typeSpecifier();
    const TypeSpecifier& typeSpecifier() const;
    TypeName& getTypeName();
    const TypeName& getTypeName() const;

private:
    TypeName typeName;
    translation_unit::Context context;
};

} // namespace ast

#endif
