#include "CodeGeneratingVisitor.h"

#include "Instruction.h"
#include "symbols/AddressPlan.h"
#include "types/TypeQuery.h"

#include "ast/Expression.h"

namespace codegen {

void CodeGeneratingVisitor::emitLvalueStore(ast::Expression& lhs, const std::string& valueName) {
    auto* lvalue = lhs.lvalueAnnotation(store_);
    if (!lvalue) {
        return;
    }
    if (const auto* bits = symbols::bitFieldOf(store_.addressPlan(&lhs))) {
        emitBitFieldInsert(lvalue->getName(), valueName, *bits, lhs.valueType(store_));
        return;
    }
    emit(ir::lvalueAssign(valueName, lvalue->getName(),
            type::memoryAccessSizeBytes(lhs.valueType(store_))));
}

} // namespace codegen
