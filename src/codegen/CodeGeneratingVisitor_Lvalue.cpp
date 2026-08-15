#include "CodeGeneratingVisitor.h"

#include "Instruction.h"
#include "symbols/AddressPlan.h"

#include "ast/Expression.h"

namespace codegen {

void CodeGeneratingVisitor::emitLvalueStore(ast::Expression& lhs, int value) {
    auto* lvalue = lhs.getLvalueSymbol(store_);
    if (!lvalue) {
        return;
    }
    if (const auto* bits = symbols::bitFieldOf(store_.addressPlan(&lhs))) {
        emitBitFieldInsert(id(*lvalue), value, *bits, lhs.getType());
        return;
    }
    emit(ir::lvalueAssign(value, id(*lvalue)));
}

} // namespace codegen
