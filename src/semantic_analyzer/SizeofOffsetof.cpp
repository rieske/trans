#include "SizeofOffsetof.h"

#include "SymbolTable.h"

#include "ast/Expression.h"
#include "ast/MemberAccess.h"
#include "ast/StringLiteralExpression.h"
#include "ast/UnaryExpression.h"
#include "symbols/AddressPlan.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {

std::optional<long> foldedSizeofBytes(ast::Expression* operand) {
    if (!operand) {
        return std::nullopt;
    }
    if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(operand)) {
        return static_cast<long>(type::sizeofStringLiteralTokenBytes(strLit->getValue()));
    }
    if (!operand->hasExpressionType()) {
        return std::nullopt;
    }
    return static_cast<long>(operand->expressionType().getSize());
}

std::optional<long> tryFoldOffsetofArrowFromNull(ast::MemberAccess* member,
        symbols::AnnotationStore& store) {
    if (!member || !member->isArrow()) {
        return std::nullopt;
    }
    const auto* field = symbols::get_if<symbols::FieldPlan>(store.addressPlan(member));
    if (!field) {
        return std::nullopt;
    }
    long baseVal = 1;
    if (!member->getBase()->evaluateConstant(baseVal) || baseVal != 0) {
        return std::nullopt;
    }
    return static_cast<long>(field->fieldOffsetBytes);
}

void applySizeofResult(ast::UnaryExpression& expression, symbols::AnnotationStore& store,
        SymbolTable& symbolTable) {
    // C: sizeof yields size_t (unsigned); signed breaks unsigned_add_overflows.
    if (!expression.getOperandExpression()->hasResult(store)) {
        expression.setTypeAndResult(store, symbolTable.createTemporarySymbol(type::unsignedLong()));
        return;
    }
    if (auto bytes = foldedSizeofBytes(expression.getOperandExpression())) {
        expression.setFoldedInteger(*bytes);
    }
    expression.setTypeAndResult(store, symbolTable.createTemporarySymbol(type::unsignedLong()));
}

bool tryApplyOffsetofFold(ast::UnaryExpression& expression, ast::Expression* operand,
        symbols::AnnotationStore& store, SymbolTable& symbolTable) {
    auto* member = dynamic_cast<ast::MemberAccess*>(operand);
    if (!member) {
        return false;
    }
    auto off = tryFoldOffsetofArrowFromNull(member, store);
    if (!off) {
        return false;
    }
    expression.setFoldedInteger(*off);
    expression.setTypeAndResult(store, symbolTable.createTemporarySymbol(type::signedInteger()));
    return true;
}

} // namespace semantic_analyzer
