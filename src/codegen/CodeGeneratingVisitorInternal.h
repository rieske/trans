#ifndef CODEGENERATINGVISITOR_INTERNAL_H_
#define CODEGENERATINGVISITOR_INTERNAL_H_

// Shared helpers for CodeGeneratingVisitor translation units.

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "ast/Expression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/Operator.h"
#include "quadruples/Add.h"
#include "quadruples/AddressOf.h"
#include "quadruples/And.h"
#include "quadruples/AssignConstant.h"
#include "quadruples/Div.h"
#include "quadruples/FieldAccess.h"
#include "quadruples/LvalueAssign.h"
#include "quadruples/Mod.h"
#include "quadruples/Mul.h"
#include "quadruples/Or.h"
#include "quadruples/Quadruple.h"
#include "quadruples/Shl.h"
#include "quadruples/Shr.h"
#include "quadruples/Sub.h"
#include "quadruples/Xor.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "Value.h"

namespace codegen {
namespace code_gen_detail {

inline void materializeArrayDecay(ast::Expression& expr,
        std::vector<std::unique_ptr<Quadruple>>& instructions) {
    if (const std::string* arrayName = expr.getArrayDecaySource()) {
        instructions.push_back(std::make_unique<AddressOf>(
                *arrayName, expr.getResultSymbol()->getName()));
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

inline Type codegenValueType(const type::Type& t) {
    return type::isFloating(t) ? Type::FLOATING : Type::INTEGRAL;
}

// Stack Value signedness — canonical policy in type::valueIsSigned.
inline bool codegenValueIsSigned(const type::Type& t) {
    return type::valueIsSigned(t);
}

// Resolve array base for IndexAddress: prefer object address lvalue when present.
inline void resolveArrayBase(std::string& baseName, bool& baseIsArray, ast::Expression* baseExpr) {
    if (baseIsArray && baseExpr) {
        if (auto* baseAddress = baseExpr->getLvalueSymbol()) {
            baseName = baseAddress->getName();
            baseIsArray = false;
        }
    }
}

// Emit StructFieldInit stores for local aggregate / compound-literal init.
inline void emitStructFieldInits(std::vector<std::unique_ptr<Quadruple>>& instructions,
        const std::string& baseName,
        const std::vector<ast::StructFieldInit>& fields) {
    for (const auto& field : fields) {
        if (field.zeroInitialize) {
            instructions.push_back(std::make_unique<AssignConstant>("0", field.source->getName()));
        } else if (field.constantValue) {
            instructions.push_back(std::make_unique<AssignConstant>(
                    *field.constantValue, field.source->getName()));
        }
        if (field.addressOfOperand) {
            // Array designator for a pointer member: store &arr, not arr's words.
            instructions.push_back(std::make_unique<AddressOf>(
                    *field.addressOfOperand, field.source->getName()));
        }
        instructions.push_back(std::make_unique<FieldAddress>(
                baseName, field.offsetBytes, field.address->getName(), false));
        // Store width is the destination field type (address pointee), not the
        // initializer expression type. Char constant 'w' is 1-byte typed but
        // int short_name (git parse-options) needs a full int/word store.
        const type::Type storeType = field.address->getType().isPointer()
                ? field.address->getType().dereference()
                : field.source->getType();
        instructions.push_back(std::make_unique<LvalueAssign>(
                field.source->getName(), field.address->getName(),
                type::memoryAccessSizeBytes(storeType)));
    }
}

// Emit left op right → result for binary arithmetic / bitwise / shift ops.
// Used by arithmetic, bitwise, shift, and compound-assignment visitors.
inline void emitBinaryOp(std::vector<std::unique_ptr<Quadruple>>& instructions,
        ast::OperatorKind kind,
        const std::string& left,
        const std::string& right,
        const std::string& result,
        bool unsignedDiv = false,
        bool logicalShr = false) {
    using ast::OperatorKind;
    switch (kind) {
    case OperatorKind::Add:
        instructions.push_back(std::make_unique<Add>(left, right, result));
        break;
    case OperatorKind::Sub:
        instructions.push_back(std::make_unique<Sub>(left, right, result));
        break;
    case OperatorKind::Mul:
        instructions.push_back(std::make_unique<Mul>(left, right, result));
        break;
    case OperatorKind::Div:
        instructions.push_back(std::make_unique<Div>(left, right, result, unsignedDiv));
        break;
    case OperatorKind::Mod:
        instructions.push_back(std::make_unique<Mod>(left, right, result, unsignedDiv));
        break;
    case OperatorKind::BitAnd:
        instructions.push_back(std::make_unique<And>(left, right, result));
        break;
    case OperatorKind::BitOr:
        instructions.push_back(std::make_unique<Or>(left, right, result));
        break;
    case OperatorKind::BitXor:
        instructions.push_back(std::make_unique<Xor>(left, right, result));
        break;
    case OperatorKind::Shl:
        instructions.push_back(std::make_unique<Shl>(left, right, result));
        break;
    case OperatorKind::Shr:
        instructions.push_back(std::make_unique<Shr>(left, right, result, logicalShr));
        break;
    default:
        throw std::runtime_error { "emitBinaryOp: not a binary arithmetic/bitwise/shift op" };
    }
}

} // namespace code_gen_detail

using code_gen_detail::materializeArrayDecay;
using code_gen_detail::pointerIncrementAmount;
using code_gen_detail::codegenValueType;
using code_gen_detail::codegenValueIsSigned;
using code_gen_detail::resolveArrayBase;
using code_gen_detail::emitStructFieldInits;
using code_gen_detail::emitBinaryOp;

} // namespace codegen

#endif // CODEGENERATINGVISITOR_INTERNAL_H_
