#ifndef CODEGENERATINGVISITOR_INTERNAL_H_
#define CODEGENERATINGVISITOR_INTERNAL_H_

// Shared helpers for CodeGeneratingVisitor translation units.

#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "CodeGeneratingVisitor.h"
#include "Instruction.h"

#include "ast/Expression.h"
#include "ast/Operator.h"
#include "types/Type.h"
#include "types/ObjectAbi.h"
#include "symbols/AnnotationStore.h"
#include "types/TypeQuery.h"
#include "Value.h"
#include "ValueKind.h"

namespace codegen {
namespace code_gen_detail {

inline void materializeArrayDecay(ast::Expression& expr,
        CodeGeneratingVisitor& cg,
        symbols::AnnotationStore& store) {
    const auto* plan = store.addressPlan(&expr);
    if (!plan) {
        return;
    }
    if (const auto* decay = symbols::get_if<symbols::ArrayDecayPlan>(plan)) {
        cg.emit(ir::addressOf(
                decay->objectName, expr.result(store)->getName()));
    }
}

// Pointer ++/-- advances by pointee size (C 6.5.6).
inline int pointerIncrementAmount(const type::Type& operandType) {
    if (!operandType.isPointer()) {
        return 1;
    }
    return type::pointerElementStride(operandType);
}

inline ValueKind codegenValueKind(const type::Type& t) {
    return valueKindFromCType(t);
}

// Caller must generateExpression(base) [and index] first.
inline symbols::AddressBaseMode addressModeOf(const symbols::ValueEntry& base) {
    return base.getType().isPointer()
            ? symbols::AddressBaseMode::PointerValue
            : symbols::AddressBaseMode::LeaObject;
}

inline void emitFieldFromPlan(const symbols::FieldPlan& field,
        ast::Expression& base,
        const std::string& destName,
        CodeGeneratingVisitor& cg,
        symbols::AnnotationStore& store) {
    auto* baseSym = base.addressSymbol(store);
    cg.emit(ir::fieldAddress(
            baseSym->getName(),
            field.fieldOffsetBytes,
            destName,
            addressModeOf(*baseSym)));
}

inline void emitIndexFromPlan(const symbols::IndexPlan& idx,
        ast::Expression& base,
        const std::string& indexResultName,
        const std::string& destName,
        CodeGeneratingVisitor& cg,
        symbols::AnnotationStore& store) {
    auto* baseSym = base.addressSymbol(store);
    cg.emit(ir::indexAddress(
            baseSym->getName(),
            indexResultName,
            idx.elementSize,
            destName,
            addressModeOf(*baseSym)));
}

inline void requireTemps(const symbols::FieldInitTemps& temps, const char* what) {
    if (!temps.source || !temps.address) {
        throw std::logic_error { std::string(what) + " missing SA temps" };
    }
}

inline void emitZeroSpan(CodeGeneratingVisitor& cg, const std::string& baseName,
        const symbols::ZeroSpanInit& z, std::string& sharedZeroName) {
    requireTemps(z.temps, "ZeroSpanInit");
    if (sharedZeroName.empty()) {
        sharedZeroName = z.temps.source->getName();
        cg.emit(ir::assignConstant("0", sharedZeroName));
    }
    int remaining = z.zeroSpanBytes;
    int off = 0;
    while (remaining > 0) {
        const int chunk = remaining >= 8 ? 8 : (remaining >= 4 ? 4 : (remaining >= 2 ? 2 : 1));
        cg.emit(ir::fieldAddress(baseName, z.offsetBytes + off, z.temps.address->getName(),
                symbols::AddressBaseMode::LeaObject));
        cg.emit(ir::lvalueAssign(sharedZeroName, z.temps.address->getName(), chunk));
        off += chunk;
        remaining -= chunk;
    }
}

inline void emitTypedStore(CodeGeneratingVisitor& cg, const std::string& baseName, int offsetBytes,
        const type::Type& storeType, const std::string& valueName, const symbols::FieldInitTemps& temps,
        const std::optional<type::BitField>& bits = {}) {
    requireTemps(temps, "StructFieldInit store");
    if (storeType.isVoid()) {
        throw std::logic_error { "StructFieldInit store row missing storeType" };
    }
    cg.emit(ir::fieldAddress(baseName, offsetBytes, temps.address->getName(),
            symbols::AddressBaseMode::LeaObject));
    if (bits) {
        cg.emitBitFieldInsert(temps.address->getName(), valueName, *bits, storeType);
        return;
    }
    cg.emit(ir::lvalueAssign(valueName, temps.address->getName(),
            type::memoryAccessSizeBytes(storeType)));
}

inline void emitStringBytes(CodeGeneratingVisitor& cg, const std::string& baseName,
        const symbols::StringBytesInit& s) {
    requireTemps(s.temps, "StringBytesInit");
    const std::string valueName = s.temps.source->getName();
    const std::string addrName = s.temps.address->getName();
    int off = 0;
    while (off < s.sizeBytes) {
        const int remaining = s.sizeBytes - off;
        const int chunk = remaining >= 8 ? 8 : (remaining >= 4 ? 4 : 1);
        unsigned long long word = 0;
        for (int i = 0; i < chunk; ++i) {
            const int idx = off + i;
            const unsigned char b = (idx < static_cast<int>(s.bytes.size()))
                    ? s.bytes[static_cast<std::size_t>(idx)] : 0;
            word |= static_cast<unsigned long long>(b) << (8 * i);
        }
        cg.emit(ir::assignConstant(std::to_string(static_cast<long long>(word)), valueName));
        cg.emit(ir::fieldAddress(baseName, s.offsetBytes + off, addrName,
                symbols::AddressBaseMode::LeaObject));
        cg.emit(ir::lvalueAssign(valueName, addrName, chunk));
        off += chunk;
    }
}

// Emit StructFieldInit stores for local aggregate / compound-literal init.
// SA always provides source + address temps (stable frame vs CL homes).
inline void emitStructFieldInits(CodeGeneratingVisitor& cg,
        const std::string& baseName,
        const std::vector<symbols::StructFieldInit>& fields) {
    std::string sharedZeroName;
    for (const auto& field : fields) {
        std::visit([&](const auto& arm) {
            using T = std::decay_t<decltype(arm)>;
            if constexpr (std::is_same_v<T, symbols::ZeroSpanInit>) {
                emitZeroSpan(cg, baseName, arm, sharedZeroName);
            } else if constexpr (std::is_same_v<T, symbols::ConstantStoreInit>) {
                requireTemps(arm.temps, "ConstantStoreInit");
                cg.emit(ir::assignConstant(arm.constantValue, arm.temps.source->getName()));
                emitTypedStore(cg, baseName, arm.offsetBytes, arm.storeType,
                        arm.temps.source->getName(), arm.temps, arm.bitField);
            } else if constexpr (std::is_same_v<T, symbols::AddressOfStoreInit>) {
                requireTemps(arm.temps, "AddressOfStoreInit");
                cg.emit(ir::addressOf(arm.addressOfOperand, arm.temps.source->getName()));
                emitTypedStore(cg, baseName, arm.offsetBytes, arm.storeType,
                        arm.temps.source->getName(), arm.temps);
            } else if constexpr (std::is_same_v<T, symbols::ValueStoreInit>) {
                requireTemps(arm.temps, "ValueStoreInit");
                emitTypedStore(cg, baseName, arm.offsetBytes, arm.storeType,
                        arm.temps.source->getName(), arm.temps, arm.bitField);
            } else if constexpr (std::is_same_v<T, symbols::StringBytesInit>) {
                emitStringBytes(cg, baseName, arm);
            }
        }, field);
    }
}

// Dual-type array object results (multi-dim rows / member arrays): keep the
// address in result; otherwise load the scalar through the address.
inline void emitAddressResultOrLoad(CodeGeneratingVisitor& cg,
        const std::string& addrName,
        const std::string& resultName,
        bool keepAddress) {
    if (keepAddress) {
        if (addrName != resultName) {
            cg.emit(ir::assign(addrName, resultName));
        }
        return;
    }
    cg.emit(ir::dereference(addrName, addrName, resultName));
}

// Field/Index visits leave the address in Lvalue. Load (or keep the address
// for array objects) when the expression is used as a value.
inline void materializeFieldIndexLoad(ast::Expression& expr,
        CodeGeneratingVisitor& cg,
        symbols::AnnotationStore& store) {
    const auto* plan = store.addressPlan(&expr);
    if (!plan) {
        return;
    }
    const auto* field = symbols::get_if<symbols::FieldPlan>(plan);
    if (!field && !symbols::get_if<symbols::IndexPlan>(plan)) {
        return;
    }
    if (!expr.hasResult(store) || !expr.lvalueAnnotation(store)) {
        return;
    }
    const std::string addrName = expr.lvalueAnnotation(store)->getName();
    const std::string resultName = expr.result(store)->getName();
    emitAddressResultOrLoad(cg, addrName, resultName, expr.holdsAggregateAddress());
    if (field && !expr.holdsAggregateAddress() && field->isBitField()) {
        cg.emitBitFieldExtract(resultName, resultName, *field->bitField);
    }
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

// 16-byte integer * / % call libgcc; one-word falls through to emitBinaryOp.
inline void emitIntegerMulDiv(CodeGeneratingVisitor& cg,
        ast::OperatorKind kind,
        const std::string& left,
        const std::string& right,
        const std::string& result,
        const type::Type& resultType,
        bool unsignedDiv = false) {
    using ast::OperatorKind;
    if (type::isIntegral(resultType) && type::object_abi::valueWords(resultType.getSize()) > 1) {
        const char* helper = "__multi3";
        if (kind == OperatorKind::Div) {
            helper = type::valueIsSigned(resultType) ? "__divti3" : "__udivti3";
        } else if (kind == OperatorKind::Mod) {
            helper = type::valueIsSigned(resultType) ? "__modti3" : "__umodti3";
        }
        cg.emit(ir::argument(left));
        cg.emit(ir::argument(right));
        cg.emit(ir::call(helper));
        cg.emit(ir::retrieve(result));
        return;
    }
    emitBinaryOp(cg, kind, left, right, result, unsignedDiv);
}

} // namespace code_gen_detail

using code_gen_detail::materializeArrayDecay;
using code_gen_detail::materializeFieldIndexLoad;
using code_gen_detail::pointerIncrementAmount;
using code_gen_detail::codegenValueKind;
using code_gen_detail::emitFieldFromPlan;
using code_gen_detail::emitIndexFromPlan;
using code_gen_detail::emitStructFieldInits;
using code_gen_detail::emitAddressResultOrLoad;
using code_gen_detail::emitBinaryOp;
using code_gen_detail::emitIntegerMulDiv;

} // namespace codegen

#endif // CODEGENERATINGVISITOR_INTERNAL_H_
