#include "CodeGeneratingVisitor.h"

#include "Instruction.h"
#include "symbols/AddressPlan.h"

#include "ast/Expression.h"

namespace codegen {

void CodeGeneratingVisitor::emitLvalueStore(ast::Expression& lhs, const std::string& valueName) {
    auto* lvalue = lhs.getLvalueSymbol(store_);
    if (!lvalue) {
        return;
    }
    if (const auto* bits = symbols::bitFieldOf(store_.addressPlan(&lhs))) {
        emitBitFieldInsert(lvalue->getName(), valueName, *bits, lhs.getType());
        return;
    }
    emit(ir::lvalueAssign(valueName, lvalue->getName()));
}

} // namespace codegen
