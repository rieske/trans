#include "SemanticAnalysisVisitorInternal.h"

#include <cassert>
#include <optional>

#include "ast/IdentifierExpression.h"

namespace semantic_analyzer {

namespace {

void setFunctionDesignator(ast::IdentifierExpression& identifier, SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
    type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
    auto addr = symbolTable.createTemporarySymbol(type::pointer(fnType));
    identifier.setFunctionDesignatorResult(addr);
    symbols::FunctionDesignatorPlan plan;
    plan.functionName = functionEntry.getName();
    plan.addressTempName = addr.getName();
    store.setAddressPlan(&identifier, symbols::AddressPlan { plan });
    // form ⇒ plan: designator form is never set without a store plan.
    assert(identifier.holdsFunctionDesignator());
    assert(symbols::get_if<symbols::FunctionDesignatorPlan>(store.addressPlan(&identifier)));
}

// Resolved call target: type for arity/return; CallPlan shape for codegen.
struct Callee {
    symbols::CallPlan::Kind kind;
    std::string calleeName;
    type::Function type;
    translation_unit::Context context;
};

std::optional<Callee> resolveCallee(ast::FunctionCall& functionCall, SymbolTable& symbolTable,
        symbols::AnnotationStore& store, std::string& errorDisplay) {
    auto* operandSym = functionCall.operandSymbol();
    type::Type operandType = operandSym->getType();
    auto* operandExpr = functionCall.getOperandExpression();

    // Designator identity lives on the store plan (label + temp). Form is the cheap tag.
    if (operandExpr->holdsFunctionDesignator()) {
        const auto* addrPlan = store.addressPlan(operandExpr);
        const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(addrPlan);
        // Invariant: FunctionDesignator form always has a FunctionDesignatorPlan.
        assert(d && "designator form without FunctionDesignatorPlan on the store");
        if (!d) {
            errorDisplay = operandSym->getName();
            return std::nullopt;
        }
        auto entry = symbolTable.findFunction(d->functionName);
        return Callee {
            symbols::CallPlan::Kind::Direct,
            d->functionName,
            entry.getType(),
            entry.getContext(),
        };
    }

    if (type::isPointerToBareFunction(operandType)) {
        type::Type pointee = operandType.dereference();
        return Callee {
            symbols::CallPlan::Kind::Indirect,
            operandSym->getName(),
            pointee.getFunction(),
            functionCall.getContext(),
        };
    }

    if (auto* id = dynamic_cast<ast::IdentifierExpression*>(operandExpr)) {
        errorDisplay = id->getIdentifier();
    } else {
        errorDisplay = unscopedSymbolName(operandSym->getName());
    }
    return std::nullopt;
}

} // namespace

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    if (!functionCall.hasOperandSymbol()) {
        return;
    }

    std::string errorDisplay;
    auto resolved = resolveCallee(functionCall, symbolTable, annotations(), errorDisplay);
    if (!resolved) {
        semanticError("called object `" + errorDisplay + "` is not a function", functionCall.getContext());
        return;
    }
    const Callee& callee = *resolved;

    // Type-check args before publishing CallPlan (avoid half-applied call shape).
    auto& arguments = functionCall.getArgumentList();
    for (auto& argument : arguments) {
        if (argument->hasResultSymbol()) {
            rejectFunctionValue(argument->getResultSymbol()->getType(), functionCall.getContext());
        }
    }

    const auto declaredArguments = callee.type.getArguments();
    const bool arityOk = arguments.size() == declaredArguments.size();
    const bool externalVarargs = callee.context == externalContext();

    if (!arityOk && !externalVarargs) {
        semanticError("no match for function " + type::function(callee.type.getReturnType(),
                declaredArguments).to_string(), functionCall.getContext());
        return;
    }

    if (arityOk) {
        for (std::size_t i { 0 }; i < arguments.size(); ++i) {
            if (!arguments.at(i)->hasResultSymbol()) {
                return;
            }
            typeCheck(arguments.at(i)->getResultSymbol()->getType(), declaredArguments.at(i),
                    functionCall.getContext());
        }
    }

    symbols::CallPlan plan;
    plan.kind = callee.kind;
    plan.calleeName = callee.calleeName;
    annotations().setCallPlan(&functionCall, plan);

    auto returnType = callee.type.getReturnType();
    if (!returnType.isVoid()) {
        functionCall.setResultSymbol(symbolTable.createTemporarySymbol(returnType));
    }
}

void SemanticAnalysisVisitor::visit(ast::IdentifierExpression& identifier) {
    const std::string& name = identifier.getIdentifier();

    // insertFunction always registers a global value symbol of function type.
    if (!symbolTable.hasSymbol(name)) {
        semanticError("symbol `" + name + "` is not defined", identifier.getContext());
        return;
    }
    auto entry = symbolTable.lookup(name);
    if (type::isBareFunction(entry.getType())) {
        setFunctionDesignator(identifier, symbolTable, annotations());
        return;
    }
    identifier.setResultSymbol(entry);
}

} // namespace semantic_analyzer
