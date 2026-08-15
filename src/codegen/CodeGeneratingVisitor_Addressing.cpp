#include "CodeGeneratingVisitor.h"

#include <string>

#include "types/TypeQuery.h"

#include "ast/Expression.h"
#include "symbols/ValueEntry.h"

namespace codegen {

void CodeGeneratingVisitor::emitArrayObjectAddress(const symbols::ValueEntry& object,
        int dest) {
    if (type::hasRuntimeSize(object.getType())) {
        emit(ir::assign(id(object), dest));
        return;
    }
    emit(ir::addressOf(id(object), dest));
}

void CodeGeneratingVisitor::emitSizeofProduct(const type::Type& measured, int result) {
    if (!type::hasComputableRuntimeSize(measured)) {
        return;
    }
    bool haveProduct = false;
    auto mulFactor = [&](int factor) {
        if (!haveProduct) {
            emit(ir::assign(factor, result));
            haveProduct = true;
            return;
        }
        emit(ir::mul(result, factor, result));
    };
    auto emitConst = [&](int n) {
        const int scratch = addScratchValue(type::signedInteger());
        emit(ir::assignConstant(id(std::to_string(n)), scratch));
        mulFactor(scratch);
    };
    type::Type t = measured;
    while (t.isArray()) {
        if (t.isVariableArray()) {
            auto bound = t.variableBound();
            bound->accept(*this);
            mulFactor(convertedResult(*bound));
        } else if (!t.isIncompleteArray()) {
            emitConst(t.getArraySize());
        }
        t = t.getElementType();
    }
    if (!type::hasRuntimeSize(t)) {
        emitConst(t.getSize());
    }
}

CodeGeneratingVisitor::ScaledIndex CodeGeneratingVisitor::scaleIndex(const type::Type& objectType,
        int indexName, int constantStrideBytes) {
    if (!type::hasComputableRuntimeSize(objectType)) {
        return {indexName, constantStrideBytes};
    }
    const int bytes = addScratchValue(type::signedInteger());
    emitSizeofProduct(objectType, bytes);
    emit(ir::mul(indexName, bytes, bytes));
    return {bytes, 1};
}

} // namespace codegen
