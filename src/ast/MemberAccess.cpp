#include "MemberAccess.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

MemberAccess::MemberAccess(std::unique_ptr<Expression> base, std::string memberName, bool arrow,
        translation_unit::Context context) :
        base { std::move(base) },
        memberName { std::move(memberName) },
        arrow { arrow },
        context { context }
{
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

void MemberAccess::setMemberOffset(int offsetBytes) {
    memberOffsetBytes = offsetBytes;
}

int MemberAccess::getMemberOffset() const {
    return memberOffsetBytes;
}

void MemberAccess::setBaseResultSymbol(semantic_analyzer::ValueEntry symbol) {
    baseResultSymbol = std::make_unique<semantic_analyzer::ValueEntry>(symbol);
}

semantic_analyzer::ValueEntry* MemberAccess::getBaseResultSymbol() const {
    return baseResultSymbol.get();
}

void MemberAccess::setFieldAddressSymbol(semantic_analyzer::ValueEntry symbol) {
    fieldAddressSymbol = std::make_unique<semantic_analyzer::ValueEntry>(symbol);
}

semantic_analyzer::ValueEntry* MemberAccess::getFieldAddressSymbol() const {
    return fieldAddressSymbol.get();
}

semantic_analyzer::ValueEntry* MemberAccess::getLvalueSymbol() const {
    return fieldAddressSymbol.get();
}

} // namespace ast
