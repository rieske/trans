#include "SemanticAnalysisVisitorInternal.h"

#include "ast/IdentifierExpression.h"

namespace semantic_analyzer {

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    if (!functionCall.hasOperandSymbol()) {
        return;
    }

    const auto symbolName = functionCall.operandSymbol()->getName();
    const auto displayName = unscopedSymbolName(symbolName);
    if (symbolName != displayName || !symbolTable.hasFunction(displayName)) {
        semanticError("called object `" + displayName + "` is not a function", functionCall.getContext());
        return;
    }

    auto functionSymbol = symbolTable.findFunction(displayName);
    functionCall.setSymbol(functionSymbol);
    symbols::CallPlan plan;
    plan.kind = symbols::CallPlan::Kind::Normal;
    plan.indirect = false;
    plan.calleeName = displayName;
    annotations().setCallPlan(&functionCall, plan);

    auto& arguments = functionCall.getArgumentList();
    for (auto& argument : arguments) {
        if (argument->hasResultSymbol()) {
            rejectFunctionValue(argument->getResultSymbol()->getType(), functionCall.getContext());
        }
    }

    if (arguments.size() == functionSymbol.argumentCount()) {
        auto declaredArguments = functionSymbol.arguments();
        for (std::size_t i { 0 }; i < arguments.size(); ++i) {
            if (!arguments.at(i)->hasResultSymbol()) {
                return;
            }
            typeCheck(arguments.at(i)->getResultSymbol()->getType(), declaredArguments.at(i),
                    functionCall.getContext());
        }
        auto returnType = functionSymbol.returnType();
        if (!returnType.isVoid()) {
            functionCall.setResultSymbol(symbolTable.createTemporarySymbol(returnType));
        }
    } else if (functionSymbol.getContext() == externalContext()) {
        auto returnType = functionSymbol.returnType();
        if (!returnType.isVoid()) {
            functionCall.setResultSymbol(symbolTable.createTemporarySymbol(returnType));
        }
    } else {
        semanticError("no match for function " + functionSymbol.getType().to_string(), functionCall.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::IdentifierExpression& identifier) {
    if (symbolTable.hasSymbol(identifier.getIdentifier())) {
        identifier.setResultSymbol(symbolTable.lookup(identifier.getIdentifier()));
    } else {
        semanticError("symbol `" + identifier.getIdentifier() + "` is not defined", identifier.getContext());
    }
}

} // namespace semantic_analyzer
