#include "SemanticAnalysisVisitorInternal.h"

#include "ast/IdentifierExpression.h"

namespace semantic_analyzer {

namespace {

void setFunctionDesignator(ast::IdentifierExpression& identifier, SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
    type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
    auto addr = symbolTable.createTemporarySymbol(type::pointer(fnType));
    identifier.setFunctionDesignatorResult(addr, functionEntry.getName());
    symbols::FunctionDesignatorPlan plan;
    plan.functionName = functionEntry.getName();
    plan.addressTempName = addr.getName();
    store.setAddressPlan(&identifier, symbols::AddressPlan { plan });
}

struct Callee {
    bool indirect { false };
    std::string calleeName;
    FunctionEntry symbol { "", type::function(type::voidType()).getFunction(), translation_unit::Context { "", 0 } };
};

bool resolveCallee(ast::FunctionCall& functionCall, SymbolTable& symbolTable, Callee& out,
        std::string& errorDisplay) {
    auto* operandSym = functionCall.operandSymbol();
    type::Type operandType = operandSym->getType();
    auto* operandExpr = functionCall.getOperandExpression();

    if (operandExpr->holdsFunctionDesignator()) {
        // Designator name is always a registered function (set in setFunctionDesignator).
        const std::string& designatorName = operandExpr->functionDesignatorName();
        out.indirect = false;
        out.calleeName = designatorName;
        out.symbol = symbolTable.findFunction(designatorName);
        return true;
    }

    if (type::isPointerToBareFunction(operandType)) {
        type::Type pointee = operandType.dereference();
        out.indirect = true;
        out.calleeName = operandSym->getName();
        out.symbol = FunctionEntry { operandSym->getName(), pointee.getFunction(), functionCall.getContext() };
        return true;
    }

    if (auto* id = dynamic_cast<ast::IdentifierExpression*>(operandExpr)) {
        errorDisplay = id->getIdentifier();
    } else {
        errorDisplay = unscopedSymbolName(operandSym->getName());
    }
    return false;
}

} // namespace

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    if (!functionCall.hasOperandSymbol()) {
        return;
    }

    Callee callee;
    std::string errorDisplay;
    if (!resolveCallee(functionCall, symbolTable, callee, errorDisplay)) {
        semanticError("called object `" + errorDisplay + "` is not a function", functionCall.getContext());
        return;
    }

    functionCall.setSymbol(callee.symbol);
    symbols::CallPlan plan;
    plan.kind = symbols::CallPlan::Kind::Normal;
    plan.indirect = callee.indirect;
    plan.calleeName = callee.calleeName;
    annotations().setCallPlan(&functionCall, plan);

    auto& arguments = functionCall.getArgumentList();
    for (auto& argument : arguments) {
        if (argument->hasResultSymbol()) {
            rejectFunctionValue(argument->getResultSymbol()->getType(), functionCall.getContext());
        }
    }

    if (arguments.size() == callee.symbol.argumentCount()) {
        auto declaredArguments = callee.symbol.arguments();
        for (std::size_t i { 0 }; i < arguments.size(); ++i) {
            if (!arguments.at(i)->hasResultSymbol()) {
                return;
            }
            const auto& declaredArgument = declaredArguments.at(i);
            const auto& actualArgument = arguments.at(i)->getResultSymbol();
            typeCheck(actualArgument->getType(), declaredArgument, functionCall.getContext());
        }

        auto returnType = callee.symbol.returnType();
        if (!returnType.isVoid()) {
            functionCall.setResultSymbol(symbolTable.createTemporarySymbol(returnType));
        }
    } else if (callee.symbol.getContext() == externalContext()) {
        auto returnType = callee.symbol.returnType();
        if (!returnType.isVoid()) {
            functionCall.setResultSymbol(symbolTable.createTemporarySymbol(returnType));
        }
    } else {
        semanticError("no match for function " + callee.symbol.getType().to_string(), functionCall.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::IdentifierExpression& identifier) {
    const std::string& name = identifier.getIdentifier();

    // insertFunction always registers a global value symbol, so hasFunction implies hasSymbol.
    if (symbolTable.hasSymbol(name)) {
        auto entry = symbolTable.lookup(name);
        if (type::isBareFunction(entry.getType()) && symbolTable.hasFunction(name)) {
            setFunctionDesignator(identifier, symbolTable, annotations());
            return;
        }
        identifier.setResultSymbol(entry);
        return;
    }

    semanticError("symbol `" + name + "` is not defined", identifier.getContext());
}

} // namespace semantic_analyzer
