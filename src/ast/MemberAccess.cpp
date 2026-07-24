#include "MemberAccess.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
MemberAccess::MemberAccess(std::unique_ptr<Expression> base, std::string memberName, bool arrow, translation_unit::Context context)
    : base{std::move(base)}, memberName{std::move(memberName)}, arrow{arrow}, context{context} { lval = true; }
void MemberAccess::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
translation_unit::Context MemberAccess::getContext() const { return context; }
Expression* MemberAccess::getBase() const { return base.get(); }
const std::string& MemberAccess::getMemberName() const { return memberName; }
bool MemberAccess::isArrow() const { return arrow; }
void MemberAccess::setMemberOffset(int offsetBytes) { memberOffsetBytes = offsetBytes; }
int MemberAccess::getMemberOffset() const { return memberOffsetBytes; }
void MemberAccess::setBaseResultSymbol(symbols::ValueEntry symbol) {
    symbols::AnnotationStore::current().setValue(this, symbols::ValueSlot::BaseResult, std::move(symbol));
}
symbols::ValueEntry* MemberAccess::getBaseResultSymbol() const {
    return symbols::AnnotationStore::current().value(this, symbols::ValueSlot::BaseResult);
}
void MemberAccess::setFieldAddressSymbol(symbols::ValueEntry symbol) {
    symbols::AnnotationStore::current().setValue(this, symbols::ValueSlot::FieldAddress, std::move(symbol));
}
symbols::ValueEntry* MemberAccess::getFieldAddressSymbol() const {
    return symbols::AnnotationStore::current().value(this, symbols::ValueSlot::FieldAddress);
}
symbols::ValueEntry* MemberAccess::getLvalueSymbol() const { return getFieldAddressSymbol(); }
} // namespace ast
