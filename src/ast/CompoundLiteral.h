#ifndef AST_COMPOUNDLITERAL_H_
#define AST_COMPOUNDLITERAL_H_

#include <memory>

#include "Expression.h"
#include "InitializerListExpression.h"
#include "TypeSpecifier.h"

namespace ast {

// C99 (type-name){ initializer-list }. The object is an lvalue.
class CompoundLiteral: public Expression {
public:
    CompoundLiteral(TypeSpecifier typeSpecifier, std::unique_ptr<InitializerListExpression> initializer);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;

    TypeSpecifier& getTypeSpecifier();
    const TypeSpecifier& getTypeSpecifier() const;
    InitializerListExpression& initializer();
    const InitializerListExpression& initializer() const;

private:
    TypeSpecifier typeSpecifier;
    std::unique_ptr<InitializerListExpression> initializer_;
};

} // namespace ast

#endif
