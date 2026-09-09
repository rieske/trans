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
        emitBitFieldInsert(id(*lvalue), value, *bits, lhs.expressionType());
        return;
    }
    emit(ir::lvalueAssign(value, id(*lvalue)));
}

} // namespace codegen
