#ifndef COMPOUNDLITERALEXPRESSION_H_
#define COMPOUNDLITERALEXPRESSION_H_

#include <memory>
#include <vector>

#include "Expression.h"
#include "InitializedDeclarator.h"
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
    InitializerListExpression* getInitializer() const;

    bool isLval() const override;
    semantic_analyzer::ValueEntry* getLvalueSymbol() const override;

    void setObjectSymbol(semantic_analyzer::ValueEntry symbol);
    semantic_analyzer::ValueEntry* getObjectSymbol() const;

    void addStructFieldInit(StructFieldInit init);
    const std::vector<StructFieldInit>& getStructFieldInits() const;

private:
    TypeSpecifier typeSpecifier;
    std::unique_ptr<InitializerListExpression> initializer;
    translation_unit::Context context;
    std::unique_ptr<semantic_analyzer::ValueEntry> objectSymbol;
    std::vector<StructFieldInit> structFieldInits;
};

} // namespace ast

#endif // COMPOUNDLITERALEXPRESSION_H_
