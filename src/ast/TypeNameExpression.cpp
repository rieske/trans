#include "TypeNameExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

TypeNameExpression::TypeNameExpression(TypeName typeName, translation_unit::Context context) :
        typeName { std::move(typeName) },
        context { std::move(context) } {
}

void TypeNameExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context TypeNameExpression::getContext() const {
    return context;
}

TypeName& TypeNameExpression::getTypeName() {
    return typeName;
}

const TypeName& TypeNameExpression::getTypeName() const {
    return typeName;
}

} // namespace ast
