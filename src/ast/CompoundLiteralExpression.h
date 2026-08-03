#ifndef COMPOUNDLITERALEXPRESSION_H_
#define COMPOUNDLITERALEXPRESSION_H_

#include <memory>

#include "Expression.h"
#include "symbols/AnnotationStore.h"
#include "InitializerListExpression.h"
#include "TypeSpecifier.h"
#include "translation_unit/Context.h"

namespace ast {

// C99 compound literal: ( type-name ) { initializer-list }
class CompoundLiteralExpression: public Expression {
public:
    CompoundLiteralExpression(TypeSpecifier typeSpecifier,
            std::unique_ptr<InitializerListExpression> initializer,
            translation_unit::Context context);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;
    const TypeSpecifier& getTypeSpecifier() const;
    // Mutable so SA can resolve __typeof__ into a concrete type on the specifier.
    TypeSpecifier& getTypeSpecifier();
    InitializerListExpression* getInitializer() const;

    bool isLval() const override;
    symbols::ValueEntry* lvalueAnnotation(symbols::AnnotationStore& store) const override;

private:
    TypeSpecifier typeSpecifier;
    std::unique_ptr<InitializerListExpression> initializer;
    translation_unit::Context context;
};

} // namespace ast

#endif // COMPOUNDLITERALEXPRESSION_H_
