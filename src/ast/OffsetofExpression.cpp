#include "OffsetofExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

OffsetofExpression::OffsetofExpression(TypeName typeName, std::string memberName,
        translation_unit::Context context) :
        typeName { std::move(typeName) },
        memberName { std::move(memberName) },
        context { std::move(context) } {
}

void OffsetofExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context OffsetofExpression::getContext() const {
    return context;
}

TypeName& OffsetofExpression::getTypeName() {
    return typeName;
}

const TypeName& OffsetofExpression::getTypeName() const {
    return typeName;
}

const std::string& OffsetofExpression::getMemberName() const {
    return memberName;
}

void OffsetofExpression::setFoldedInteger(long value) {
    foldedInteger_ = value;
}

bool OffsetofExpression::evaluateConstant(long& value) const {
    if (!foldedInteger_) {
        return false;
    }
    value = *foldedInteger_;
    return true;
}

} // namespace ast
