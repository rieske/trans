#include "SizeofOffsetof.h"

#include "SymbolTable.h"

#include "ast/ArrayAccess.h"
#include "ast/Expression.h"
#include "ast/IdentifierExpression.h"
#include "ast/MemberAccess.h"
#include "ast/TypeNameExpression.h"
#include "ast/UnaryExpression.h"
#include "symbols/AddressPlan.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {

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
    if (!member->getBase()->foldToHostLong(baseVal) || baseVal != 0) {
        return std::nullopt;
    }
    return static_cast<long>(field->fieldOffsetBytes);
}

std::optional<std::string> applySizeof(ast::UnaryExpression& expression, bool gnuExtensions,
        symbols::AnnotationStore& store, SymbolTable& symbolTable) {
    expression.setTypeAndResult(store, symbolTable.createTemporarySymbol(type::unsignedLong()));
    ast::Expression* operand = expression.getOperandExpression();
    if (operand && operand->holdsFunctionDesignator()) {
        if (gnuExtensions) {
            expression.setFoldedInteger(1);
            return std::nullopt;
        }
        return "invalid application of sizeof to incomplete type '"
                + operand->expressionType().to_string() + "'";
    }
    if (operand && symbols::bitFieldOf(store.addressPlan(operand))) {
        return "invalid application of sizeof to a bit-field";
    }
    if (operand && operand->hasExpressionType()) {
        const type::Type measured = operand->expressionType();
        if (auto bytes = type::sizeofObject(measured, gnuExtensions)) {
            expression.setFoldedInteger(*bytes);
        } else if (!type::hasComputableRuntimeSize(measured)) {
            return "invalid application of sizeof to incomplete type '"
                    + measured.to_string() + "'";
        }
    }
    return std::nullopt;
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

void foldSizeofInBound(ast::Expression* expr, SymbolTable& symbolTable,
        ast::AbstractSyntaxTreeVisitor& visitor) {
    walkBoundExpressionTree(expr, [&](ast::Expression* node) {
        auto* unary = dynamic_cast<ast::UnaryExpression*>(node);
        if (!unary) {
            return;
        }
        ast::Expression* op = unary->getOperandExpression();
        if (dynamic_cast<ast::TypeNameExpression*>(op)) {
            unary->accept(visitor);
            return;
        }
        if (auto* id = dynamic_cast<ast::IdentifierExpression*>(op)) {
            if (symbolTable.hasSymbol(id->getIdentifier())) {
                type::Type t = symbolTable.lookup(id->getIdentifier()).getType();
                unary->setFoldedInteger(t.getSize());
            }
            return;
        }
        if (auto* access = dynamic_cast<ast::ArrayAccess*>(op)) {
            ast::Expression* base = access->getLeftOperand();
            if (auto* id = dynamic_cast<ast::IdentifierExpression*>(base)) {
                if (symbolTable.hasSymbol(id->getIdentifier())) {
                    type::Type t = symbolTable.lookup(id->getIdentifier()).getType();
                    if (t.isArray()) {
                        unary->setFoldedInteger(t.getElementType().getSize());
                    }
                }
            }
        }
    });
}

} // namespace semantic_analyzer
