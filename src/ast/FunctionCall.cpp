#include "FunctionCall.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
FunctionCall::FunctionCall(std::unique_ptr<Expression> postfixExpression, std::vector<std::unique_ptr<Expression>> argumentList)
    : SingleOperandExpression{std::move(postfixExpression), std::unique_ptr<Operator>{new Operator("()")}},
      argumentList{std::move(argumentList)} {}
void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void FunctionCall::visitArguments(AbstractSyntaxTreeVisitor& visitor) {
    for (const auto& argument : argumentList) argument->accept(visitor);
}
const std::vector<std::unique_ptr<Expression>>& FunctionCall::getArgumentList() const { return argumentList; }
void FunctionCall::setSymbol(symbols::FunctionEntry symbol) {
    symbols::AnnotationStore::current().setFunctionSymbol(this, std::move(symbol));
}
symbols::FunctionEntry* FunctionCall::getSymbol() const {
    return symbols::AnnotationStore::current().functionSymbol(this);
}
void FunctionCall::setBuiltinKind(BuiltinKind kind) { builtinKind = kind; }
FunctionCall::BuiltinKind FunctionCall::getBuiltinKind() const { return builtinKind; }
void FunctionCall::setVaArgResultType(type::Type type) { vaArgResultType = std::move(type); }
const type::Type* FunctionCall::getVaArgResultType() const {
    return vaArgResultType ? &*vaArgResultType : nullptr;
}
void FunctionCall::setIndirect(bool indirect) { indirectCall = indirect; }
bool FunctionCall::isIndirect() const { return indirectCall; }
} // namespace ast
