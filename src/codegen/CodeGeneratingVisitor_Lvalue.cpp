#include "CodeGeneratingVisitor.h"
#include "ast/AstNodes.h"

#include "codegen/IrBuilders.h"
#include "symbols/AddressPlan.h"

namespace codegen {

void CodeGeneratingVisitor::emitLvalueStore(ast::Expression& lhs, int value) {
    auto* lvalue = lhs.getLvalueSymbol(store_);
    if (!lvalue) {
        return;
    }
    if (const auto* bits = symbols::bitFieldOf(store_.addressPlan(&lhs))) {
        const type::Type unit = lhs.expressionType();
        emitBitFieldInsert(id(*lvalue), value, *bits, unit);
        // Assignment yields the stored field after promotion, not the untruncated RHS.
        const int container = addScratchValue(unit);
        emit(ir::dereference(id(*lvalue), container));
        emitBitFieldExtract(container, value, *bits);
        return;
    }
    emit(ir::lvalueAssign(value, id(*lvalue)));
}

} // namespace codegen
