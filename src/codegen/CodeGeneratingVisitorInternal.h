#ifndef CODEGENERATINGVISITOR_INTERNAL_H_
#define CODEGENERATINGVISITOR_INTERNAL_H_

// Shared helpers for CodeGeneratingVisitor translation units.

#include <stdexcept>
#include <string>

#include "CodeGeneratingVisitor.h"
#include "Instruction.h"

#include "ast/Expression.h"
#include "ast/Operator.h"
#include "types/Type.h"
#include "symbols/AnnotationStore.h"
#include "types/TypeQuery.h"
#include "Value.h"

namespace codegen {
namespace code_gen_detail {

inline void materializeArrayDecay(ast::Expression& expr,
        CodeGeneratingVisitor& cg,
        symbols::AnnotationStore& store) {
    if (const std::string* arrayName = store.string(&expr, symbols::StringSlot::ArrayDecaySource)) {
        cg.emit(ir::addressOf(
                *arrayName, expr.result(store)->getName()));
    }
}

// Pointer ++/-- advances by pointee size (C 6.5.6).
inline int pointerIncrementAmount(const type::Type& operandType) {
    if (!operandType.isPointer()) {
        return 1;
    }
    type::Type elem = operandType.dereference();
    int scale = elem.getSize();
    if (scale < 1) {
        return 1;
    }
    return scale;
}

inline ValueKind codegenValueKind(const type::Type& t) {
    return type::isFloating(t) ? ValueKind::FLOATING : ValueKind::INTEGRAL;
}

// Shared Field/Index materialization for visit(Member/Array) and emitAddressOf.
// Caller must generateExpression(base) [and index] first. Base name/mode come
// entirely from SA AddressBaseMode on the plan.
// Base symbol name is SA-owned on the plan (not re-derived in CG).
inline void emitFieldFromPlan(const symbols::FieldPlan& field,
        const std::string& destName,
        CodeGeneratingVisitor& cg) {
    cg.emit(ir::fieldAddress(
            field.base.name,
            field.fieldOffsetBytes,
            destName,
            field.base.mode));
}

// indexResultName is SA/CG result of the index expression (already generated).
inline void emitIndexFromPlan(const symbols::IndexPlan& idx,
        const std::string& indexResultName,
        const std::string& destName,
        CodeGeneratingVisitor& cg) {
    cg.emit(ir::indexAddress(
            idx.base.name,
            indexResultName,
            idx.elementSize,
            destName,
            idx.base.mode));
}

// Emit StructFieldInit stores for local aggregate / compound-literal init.
// Zero plan entries always use zeroSpanBytes > 0 (single span-zero path).
inline void emitStructFieldInits(CodeGeneratingVisitor& cg,
        const std::string& baseName,
        const std::vector<symbols::StructFieldInit>& fields) {
    std::string sharedZeroName;
    for (const auto& field : fields) {
        if (field.zeroSpanBytes > 0) {
            // Compact span zero: one 0 temp, then word/byte stores over the span.
            if (sharedZeroName.empty()) {
                sharedZeroName = field.source->getName();
                cg.emit(ir::assignConstant("0", sharedZeroName));
            }
            int remaining = field.zeroSpanBytes;
            int off = 0;
            while (remaining > 0) {
                const int chunk = remaining >= 8 ? 8
                        : (remaining >= 4 ? 4 : (remaining >= 2 ? 2 : 1));
                cg.emit(ir::fieldAddress(
                        baseName, field.offsetBytes + off, field.address->getName(),
                        symbols::AddressBaseMode::LeaObject));
                cg.emit(ir::lvalueAssign(
                        sharedZeroName, field.address->getName(), chunk));
                off += chunk;
                remaining -= chunk;
            }
            continue;
        }
        std::string valueName = field.source->getName();
        if (field.constantValue) {
            cg.emit(ir::assignConstant(
                    *field.constantValue, valueName));
        } else if (field.addressOfOperand) {
            cg.emit(ir::addressOf(
                    *field.addressOfOperand, field.source->getName()));
            valueName = field.source->getName();
        }
        cg.emit(ir::fieldAddress(
                baseName, field.offsetBytes, field.address->getName(),
                symbols::AddressBaseMode::LeaObject));
        const type::Type storeType = field.address->getType().isPointer()
                ? field.address->getType().dereference()
                : field.source->getType();
        cg.emit(ir::lvalueAssign(
                valueName, field.address->getName(),
                type::memoryAccessSizeBytes(storeType)));
    }
}

// Dual-type array object results (multi-dim rows / member arrays): keep the
// address in result; otherwise load the scalar through the address.
inline void emitAddressResultOrLoad(CodeGeneratingVisitor& cg,
        const std::string& addrName,
        const std::string& resultName,
        bool keepAddress,
        const type::Type& loadType) {
    if (keepAddress) {
        if (addrName != resultName) {
            cg.emit(ir::assign(addrName, resultName));
        }
        return;
    }
    cg.emit(ir::dereference(
            addrName, addrName, resultName,
            type::memoryAccessSizeBytes(loadType),
            type::memoryAccessIsSigned(loadType)));
}

// Emit left op right → result for binary arithmetic / bitwise / shift ops.
// Used by arithmetic, bitwise, shift, and compound-assignment visitors.
inline void emitBinaryOp(CodeGeneratingVisitor& cg,
        ast::OperatorKind kind,
        const std::string& left,
        const std::string& right,
        const std::string& result,
        bool unsignedDiv = false,
        bool logicalShr = false) {
    using ast::OperatorKind;
    switch (kind) {
    case OperatorKind::Add:
        cg.emit(ir::add(left, right, result));
        break;
    case OperatorKind::Sub:
        cg.emit(ir::sub(left, right, result));
        break;
    case OperatorKind::Mul:
        cg.emit(ir::mul(left, right, result));
        break;
    case OperatorKind::Div:
        cg.emit(ir::div(left, right, result, unsignedDiv));
        break;
    case OperatorKind::Mod:
        cg.emit(ir::mod(left, right, result, unsignedDiv));
        break;
    case OperatorKind::BitAnd:
        cg.emit(ir::andOp(left, right, result));
        break;
    case OperatorKind::BitOr:
        cg.emit(ir::orOp(left, right, result));
        break;
    case OperatorKind::BitXor:
        cg.emit(ir::xorOp(left, right, result));
        break;
    case OperatorKind::Shl:
        cg.emit(ir::shl(left, right, result));
        break;
    case OperatorKind::Shr:
        cg.emit(ir::shr(left, right, result, logicalShr));
        break;
    default:
        throw std::runtime_error { "emitBinaryOp: not a binary arithmetic/bitwise/shift op" };
    }
}

} // namespace code_gen_detail

using code_gen_detail::materializeArrayDecay;
using code_gen_detail::pointerIncrementAmount;
using code_gen_detail::codegenValueKind;
using code_gen_detail::emitFieldFromPlan;
using code_gen_detail::emitIndexFromPlan;
using code_gen_detail::emitStructFieldInits;
using code_gen_detail::emitAddressResultOrLoad;
using code_gen_detail::emitBinaryOp;

} // namespace codegen

#endif // CODEGENERATINGVISITOR_INTERNAL_H_
