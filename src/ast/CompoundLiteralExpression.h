#ifndef COMPOUNDLITERALEXPRESSION_H_
#define COMPOUNDLITERALEXPRESSION_H_

#include <memory>

#include "Expression.h"
#include "symbols/AnnotationStore.h"
#include "InitializerListExpression.h"
#include "TypeName.h"
#include "translation_unit/Context.h"

namespace ast {

// C99 compound literal: ( type-name ) { initializer-list }
class CompoundLiteralExpression: public Expression {
public:
    CompoundLiteralExpression(TypeName typeName,
            std::unique_ptr<InitializerListExpression> initializer,
            translation_unit::Context context);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    const TypeName& getTypeName() const;
    TypeName& getTypeName();
    InitializerListExpression* getInitializer() const;
    void setInitializer(std::unique_ptr<InitializerListExpression> initializer);

    bool isLval() const override;

private:
    TypeName typeName;
    std::unique_ptr<InitializerListExpression> initializer;
    translation_unit::Context context;
};

} // namespace ast

#endif // COMPOUNDLITERALEXPRESSION_H_
