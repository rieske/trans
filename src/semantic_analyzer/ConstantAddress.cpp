#include "ConstantAddress.h"

#include "ast/ArrayAccess.h"
#include "ast/DoubleOperandExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/MemberAccess.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TypeCast.h"
#include "ast/UnaryExpression.h"
#include "ast/Operator.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/ImmediateFormat.h"

namespace semantic_analyzer {


std::string AddressConstant::toOperand() const {
    if (byteOffset == 0) {
        return label;
    }
    return label + "+" + std::to_string(byteOffset);
}

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

bool resolveAddressConstant(ast::Expression* expr, AddressConstant& out,
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
                out.byteOffset += scale * static_cast<long>(elemSize);
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
            out.label = storageLabel(baseId);
            out.byteOffset = totalOffset;
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

} // namespace

std::string formatDataWord(unsigned long long v) {
    return util::wordImmediate(v);
}

bool tryFoldDataOperand(ast::Expression* expr, const type::Type& storeType,
        symbols::AnnotationStore& store, std::string& outOperand) {
    if (!expr) {
        return false;
    }
    expr = peelTypeCasts(expr);
    if (!expr) {
        return false;
    }
    long v = 0;
    if (expr->evaluateConstant(v)) {
        outOperand = formatDataWord(static_cast<unsigned long long>(v));
        return true;
    }
    if (!storeTypeCanHoldAddress(storeType)) {
        return false;
    }
    if (auto* str = dynamic_cast<ast::StringLiteralExpression*>(expr)) {
        outOperand = str->getConstantSymbol();
        return !outOperand.empty();
    }
    AddressConstant addrConst;
    if (resolveAddressConstant(expr, addrConst, store,
            [&](ast::IdentifierExpression* id) { return defaultStorageLabel(id, store); })) {
        outOperand = addrConst.toOperand();
        return true;
    }
    if (auto* id = dynamic_cast<ast::IdentifierExpression*>(expr)) {
        if (const std::string* fn = id->functionDesignatorName(store)) {
            outOperand = *fn;
            return true;
        }
        if (expr->hasResult(store) && expr->valueType(store).isArray()) {
            outOperand = defaultStorageLabel(id, store);
            return !outOperand.empty();
        }
    }
    return false;
}

} // namespace semantic_analyzer
