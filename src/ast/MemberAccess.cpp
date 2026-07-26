#include "MemberAccess.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

MemberAccess::MemberAccess(std::unique_ptr<Expression> base, std::string memberName, bool arrow,
        translation_unit::Context context) :
        base { std::move(base) },
        memberName { std::move(memberName) },
        arrow { arrow },
        context { std::move(context) } {
    lval = true;
}

void MemberAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context MemberAccess::getContext() const {
    return context;
}

Expression* MemberAccess::getBase() const {
    return base.get();
}

const std::string& MemberAccess::getMemberName() const {
    return memberName;
}

bool MemberAccess::isArrow() const {
    return arrow;
}

void MemberAccess::setFieldAddressSymbol(symbols::ValueEntry symbol) {
    fieldAddress = std::make_unique<symbols::ValueEntry>(std::move(symbol));
}

symbols::ValueEntry* MemberAccess::getFieldAddressSymbol() const {
    return fieldAddress.get();
}

symbols::ValueEntry* MemberAccess::getLvalueSymbol(symbols::AnnotationStore& store) const {
    (void)store;
    return fieldAddress.get();
}

} // namespace ast
