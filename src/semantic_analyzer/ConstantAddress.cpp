#include "ConstantAddress.h"

#include "ast/ArrayAccess.h"
#include "ast/DoubleOperandExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/MemberAccess.h"
#include "ast/TypeCast.h"
#include "ast/UnaryExpression.h"
#include "ast/Operator.h"
#include "types/Type.h"

namespace semantic_analyzer {

std::string AddressConstant::toOperand() const {
    if (byteOffset == 0) {
        return label;
    }
    return label + "+" + std::to_string(byteOffset);
}

std::string defaultStorageLabel(ast::IdentifierExpression* id) {
    if (!id) {
        return {};
    }
    if (id->hasFunctionDesignator()) {
        return id->getFunctionDesignator();
    }
    if (id->hasResultSymbol()) {
        return id->getResultSymbol()->getName();
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
            if (ptrSide && resolveAddressConstant(ptrSide, out, storageLabel)) {
                int elemSize = 1;
                type::Type pt = ptrSide->getType();
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
                totalOffset += member->getMemberOffset();
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
                totalOffset += index * static_cast<long>(arr->getElementSize());
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

} // namespace semantic_analyzer
