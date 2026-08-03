#ifndef TYPENAMEEXPRESSION_H_
#define TYPENAMEEXPRESSION_H_

#include "Expression.h"
#include "TypeName.h"
#include "translation_unit/Context.h"

namespace ast {

// type_name used as a sizeof operand: sizeof(T) / sizeof(T[N]) / sizeof(__typeof__(*p)).
class TypeNameExpression: public Expression {
public:
    TypeNameExpression(TypeName typeName, translation_unit::Context context);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    translation_unit::Context getContext() const override;

    TypeName& getTypeName();
    const TypeName& getTypeName() const;

private:
    TypeName typeName;
    translation_unit::Context context;
};

} // namespace ast

#endif
