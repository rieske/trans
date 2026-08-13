#include "CSNB_Internal.h"

#include "OffsetofExpression.h"

namespace ast {
namespace csnb {

namespace {

void builtinOffsetof(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    auto memberTok = context.popTerminal(); // id
    context.popTerminal(); // ,
    auto typeName = context.popTypeName();
    context.popTerminal(); // (
    auto kw = context.popTerminal(); // __builtin_offsetof
    // Seed ICE when the record is already complete; SA always re-folds.
    long folded = 0;
    bool canFold = false;
    if (auto record = typeName.tryResolve(context.environment())) {
        const type::OffsetofResult off = type::resolveOffsetof(*record, memberTok.value);
        if (off.status == type::OffsetofStatus::Ok) {
            folded = off.offsetBytes;
            canFold = true;
        }
    }
    auto expr = std::make_unique<OffsetofExpression>(
            std::move(typeName), memberTok.value, kw.context);
    if (canFold) {
        expr->setFoldedInteger(folded);
    }
    context.pushExpression(std::move(expr));
}

} // namespace


void registerBuiltinProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry) {
    int s_unary_exp = grammar.symbolId("<unary_exp>");
    int s_open_paren = grammar.symbolId("(");
    int s_close_paren = grammar.symbolId(")");
    int s_comma = grammar.symbolId(",");
    int s_type_name = grammar.symbolId("<type_name>");
    int s_identifier = grammar.symbolId("id");

    nodeCreatorRegistry[s_unary_exp][{
            grammar.symbolId("__builtin_offsetof"), s_open_paren, s_type_name,
            s_comma, s_identifier, s_close_paren }] = builtinOffsetof;
}

} // namespace csnb
} // namespace ast
