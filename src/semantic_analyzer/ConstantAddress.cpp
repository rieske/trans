#include "ConstantAddress.h"

#include "ast/ArrayAccess.h"
#include "ast/ConstantExpression.h"
#include "ast/DoubleOperandExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/MemberAccess.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TypeCast.h"
#include "ast/UnaryExpression.h"
#include "ast/Operator.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"

#include <utility>

namespace semantic_analyzer {

std::string defaultStorageLabel(ast::IdentifierExpression* id, symbols::AnnotationStore& store) {
    if (!id) {
        return {};
    }
    if (const std::string* dn = id->functionDesignatorName(store)) {
        return *dn;
    }
    if (id->hasResult(store)) {
        return id->result(store)->getName();
    }
    return id->getIdentifier();
}

ast::Expression* peelTypeCasts(ast::Expression* expr) {
    while (auto* cast = dynamic_cast<ast::TypeCast*>(expr)) {
        expr = cast->getOperandExpression();
        if (!expr) {
            return nullptr;
        }
    }
    return expr;
}

bool resolveAddressConstant(ast::Expression* expr, symbols::AddressInit& out,
        symbols::AnnotationStore& store,
        std::function<std::string(ast::IdentifierExpression*)> storageLabel) {
    if (!expr) {
        return false;
    }
    expr = peelTypeCasts(expr);
    if (!expr) {
        return false;
    }

    // ptr + n / n + ptr / ptr - n (C 6.6 address constant).
    if (auto* bin = dynamic_cast<ast::DoubleOperandExpression*>(expr)) {
        using ast::OperatorKind;
        const OperatorKind op = bin->getOperator()->getKind();
        if (op == OperatorKind::Add || op == OperatorKind::Sub) {
            long index = 0;
            ast::Expression* ptrSide = nullptr;
            long scale = 0;
            if (bin->getRightOperand()->evaluateConstant(index)) {
                ptrSide = bin->getLeftOperand();
                scale = (op == OperatorKind::Sub) ? -index : index;
            } else if (op == OperatorKind::Add && bin->getLeftOperand()->evaluateConstant(index)) {
                ptrSide = bin->getRightOperand();
                scale = index;
            }
            if (ptrSide && resolveAddressConstant(ptrSide, out, store, storageLabel)) {
                int elemSize = 1;
                type::Type pt = ptrSide->valueType(store);
                if (pt.isPointer()) {
                    elemSize = pt.dereference().getSize();
                } else if (pt.isArray()) {
                    elemSize = pt.getElementType().getSize();
                }
                if (elemSize < 1) {
                    elemSize = 1;
                }
                out.offset += scale * static_cast<long>(elemSize);
                return true;
            }
        }
    }

    if (auto* unary = dynamic_cast<ast::UnaryExpression*>(expr);
            unary && unary->getOperator()->getKind() == ast::OperatorKind::AddressOf) {
        ast::Expression* cur = unary->getOperandExpression();
        long totalOffset = 0;
        while (cur) {
            if (auto* member = dynamic_cast<ast::MemberAccess*>(cur);
                    member && !member->isArrow()) {
                const auto* field = symbols::get_if<symbols::FieldPlan>(store.addressPlan(member));
                if (!field) {
                    cur = nullptr;
                    break;
                }
                totalOffset += field->fieldOffsetBytes;
                cur = member->getBase();
                continue;
            }
            if (auto* arr = dynamic_cast<ast::ArrayAccess*>(cur)) {
                long index = -1;
                auto* subscript = arr->getRightOperand();
                if (!subscript || !subscript->evaluateConstant(index) || index < 0) {
                    cur = nullptr;
                    break;
                }
                const auto* idx = symbols::get_if<symbols::IndexPlan>(store.addressPlan(arr));
                if (!idx) {
                    cur = nullptr;
                    break;
                }
                totalOffset += index * static_cast<long>(idx->elementSize);
                cur = arr->getLeftOperand();
                continue;
            }
            break;
        }
        if (auto* baseId = dynamic_cast<ast::IdentifierExpression*>(cur)) {
            if (baseId->functionDesignatorName(store)) {
                out.symbol = storageLabel(baseId);
                out.offset = totalOffset;
                return true;
            }
            if (!baseId->hasResult(store) || !baseId->result(store)->isGlobal()) {
                return false;
            }
            out.symbol = storageLabel(baseId);
            out.offset = totalOffset;
            return true;
        }
    }
    return false;
}

namespace {

bool storeTypeCanHoldAddress(const type::Type& storeType) {
    return storeType.isPointer()
            || (type::isIntegral(storeType)
                    && storeType.getSize() >= type::object_abi::MACHINE_WORD_SIZE);
}

bool foldFloatingBits(ast::Expression* expr, util::FloatingBits& out) {
    expr = peelTypeCasts(expr);
    if (!expr) {
        return false;
    }
    if (auto* constant = dynamic_cast<ast::ConstantExpression*>(expr)) {
        if (constant->hasExpressionType() && type::isFloating(constant->expressionType())) {
            return util::floatingLiteralBits(constant->getValue(), out);
        }
        return false;
    }
    auto* unary = dynamic_cast<ast::UnaryExpression*>(expr);
    if (!unary) {
        return false;
    }
    const ast::OperatorKind op = unary->getOperator()->getKind();
    if (op != ast::OperatorKind::Add && op != ast::OperatorKind::Sub) {
        return false;
    }
    if (!foldFloatingBits(unary->getOperandExpression(), out)) {
        return false;
    }
    if (op == ast::OperatorKind::Sub) {
        const double negated = -util::decodeFloating(out.bits, out.sizeBytes);
        out.bits = util::encodeFloating(negated, out.sizeBytes);
    }
    return true;
}

bool foldNumericDataWord(ast::Expression* expr, const type::Type& dest, symbols::DataWord& outWord) {
    util::FloatingBits fp;
    if (foldFloatingBits(expr, fp)) {
        if (type::isFloating(dest)) {
            unsigned long long bits = fp.bits;
            if (fp.sizeBytes != dest.getSize()) {
                bits = util::encodeFloating(
                        util::decodeFloating(fp.bits, fp.sizeBytes), dest.getSize());
            }
            outWord = symbols::ConstantInit { static_cast<long>(bits) };
            return true;
        }
        const double decoded = util::decodeFloating(fp.bits, fp.sizeBytes);
        if (type::isBoolean(dest)) {
            outWord = symbols::ConstantInit { decoded != 0.0 };
            return true;
        }
        if (type::isIntegral(dest)) {
            outWord = symbols::ConstantInit {
                    type::convertScalarConstant(dest, static_cast<long>(decoded)) };
            return true;
        }
        return false;
    }
    long v = 0;
    if (!expr->evaluateConstant(v)) {
        return false;
    }
    if (type::isFloating(dest)) {
        outWord = symbols::ConstantInit {
                static_cast<long>(util::encodeFloating(static_cast<double>(v), dest.getSize())) };
        return true;
    }
    outWord = symbols::ConstantInit { type::convertScalarConstant(dest, v) };
    return true;
}

} // namespace

bool tryFoldDataWord(ast::Expression* expr, const type::Type& storeType,
        symbols::AnnotationStore& store, symbols::DataWord& outWord) {
    if (!expr) {
        return false;
    }
    expr = peelTypeCasts(expr);
    if (!expr) {
        return false;
    }
    if (foldNumericDataWord(expr, storeType, outWord)) {
        return true;
    }
    if (!storeTypeCanHoldAddress(storeType)) {
        return false;
    }
    if (auto* str = dynamic_cast<ast::StringLiteralExpression*>(expr)) {
        const std::string label = str->getConstantSymbol();
        if (label.empty()) {
            return false;
        }
        outWord = symbols::AddressInit { label, 0 };
        return true;
    }
    symbols::AddressInit addr;
    if (resolveAddressConstant(expr, addr, store,
            [&](ast::IdentifierExpression* id) { return defaultStorageLabel(id, store); })) {
        outWord = std::move(addr);
        return true;
    }
    if (auto* id = dynamic_cast<ast::IdentifierExpression*>(expr)) {
        if (const std::string* fn = id->functionDesignatorName(store)) {
            outWord = symbols::AddressInit { *fn, 0 };
            return true;
        }
        if (expr->hasResult(store) && expr->result(store)->isGlobal()
                && expr->valueType(store).isArray()) {
            const std::string label = defaultStorageLabel(id, store);
            if (label.empty()) {
                return false;
            }
            outWord = symbols::AddressInit { label, 0 };
            return true;
        }
    }
    return false;
}

} // namespace semantic_analyzer
