#include "SizeofOffsetof.h"

#include "SymbolTable.h"

#include "ast/ArrayAccess.h"
#include "ast/Expression.h"
#include "ast/IdentifierExpression.h"
#include "ast/MemberAccess.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TypeNameExpression.h"
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

void foldSizeofInBound(ast::Expression* expr, SymbolTable& symbolTable,
        ast::AbstractSyntaxTreeVisitor& visitor) {
    walkBoundExpressionTree(expr, [&](ast::Expression* node) {
        if (auto* off = dynamic_cast<ast::OffsetofExpression*>(node)) {
            off->accept(visitor);
            return;
        }
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
