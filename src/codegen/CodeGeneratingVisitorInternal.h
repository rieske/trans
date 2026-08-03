#ifndef CODEGENERATINGVISITOR_INTERNAL_H_
#define CODEGENERATINGVISITOR_INTERNAL_H_

// Shared helpers for CodeGeneratingVisitor translation units.

#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

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

// Product identity for SA temps: Result slots own ValueEntry copies, so equal
// temps share a name rather than a C++ object address.
inline bool sameValueTemp(const symbols::ValueEntry* a, const symbols::ValueEntry* b) {
    return a && b && a->getName() == b->getName();
}

inline void materializeArrayDecay(ast::Expression& expr,
        CodeGeneratingVisitor& cg,
        symbols::AnnotationStore& store) {
    const auto* plan = store.addressPlan(&expr);
    if (!plan) {
        return;
    }
    if (const auto* decay = symbols::get_if<symbols::ArrayDecayPlan>(plan)) {
        cg.emitArrayObjectAddress(
                decay->objectName, expr.getResultSymbol(store)->getName());
    }
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
    const auto scaled = cg.scaleIndex(idx.elementType, indexResultName, idx.elementSize);
    cg.emit(ir::indexAddress(
            baseSym->getName(),
            scaled.name,
            scaled.strideBytes,
            destName,
            idx.baseMode));
}

void emitFieldInits(CodeGeneratingVisitor& cg,
        const std::string& baseName,
        const std::vector<symbols::FieldInit>& fields);

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
    if (!expr.hasResultSymbol(store) || !expr.getLvalueSymbol(store)) {
        return;
    }
    const std::string addrName = expr.getLvalueSymbol(store)->getName();
    const std::string resultName = expr.getResultSymbol(store)->getName();
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
using code_gen_detail::codegenValueKind;
using code_gen_detail::sameValueTemp;
using code_gen_detail::emitFieldFromPlan;
using code_gen_detail::emitIndexFromPlan;
using code_gen_detail::emitFieldInits;
using code_gen_detail::emitAddressResultOrLoad;
using code_gen_detail::emitBinaryOp;
using code_gen_detail::emitIntegerMulDiv;

} // namespace codegen

#endif // CODEGENERATINGVISITOR_INTERNAL_H_
