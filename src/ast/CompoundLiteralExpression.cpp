#include "CompoundLiteralExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

CompoundLiteralExpression::CompoundLiteralExpression(TypeSpecifier typeSpecifier,
        std::unique_ptr<InitializerListExpression> initializer,
        translation_unit::Context context) :
        typeSpecifier { std::move(typeSpecifier) },
        initializer { std::move(initializer) },
        context { std::move(context) }
{
    lval = true;
}

void CompoundLiteralExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context CompoundLiteralExpression::getContext() const {
    return context;
}

const TypeSpecifier& CompoundLiteralExpression::getTypeSpecifier() const {
    return typeSpecifier;
}

InitializerListExpression* CompoundLiteralExpression::getInitializer() const {
    return initializer.get();
}

bool CompoundLiteralExpression::isLval() const {
    return true;
}

semantic_analyzer::ValueEntry* CompoundLiteralExpression::getLvalueSymbol() const {
    // Array compound literals decay via AddressOf of the object home (same as
    // named arrays). Returning the object itself would pass the first word as a
    // fake pointer (git oid-array TEST_LOOKUP compound-literal args).
    if (objectSymbol && objectSymbol->getType().isArray()) {
        return nullptr;
    }
    return objectSymbol.get();
}

void CompoundLiteralExpression::setObjectSymbol(semantic_analyzer::ValueEntry symbol) {
    objectSymbol = std::make_unique<semantic_analyzer::ValueEntry>(std::move(symbol));
}

semantic_analyzer::ValueEntry* CompoundLiteralExpression::getObjectSymbol() const {
    return objectSymbol.get();
}

void CompoundLiteralExpression::addStructFieldInit(StructFieldInit init) {
    structFieldInits.push_back(std::move(init));
}

const std::vector<StructFieldInit>& CompoundLiteralExpression::getStructFieldInits() const {
    return structFieldInits;
}

} // namespace ast
