#include "SemanticAnalysisVisitorInternal.h"

#include "builtins/BuiltinRegistry.h"

namespace semantic_analyzer {

std::optional<SemanticAnalysisVisitor::ResolvedCallee> SemanticAnalysisVisitor::resolveCallee(
        const std::string& designatorName,
        symbols::ValueEntry* operandSym,
        const type::Type& operandType,
        const translation_unit::Context& callContext) {
    if (!designatorName.empty() && symbolTable.hasFunction(designatorName)) {
        return ResolvedCallee { symbolTable.findFunction(designatorName), false };
    }
    if (operandType.isPointer()) {
        type::Type pointee = operandType.dereference();
        if (pointee.isFunction()) {
            return ResolvedCallee {
                    FunctionEntry { operandSym->getName(), pointee.getFunction(), callContext },
                    true };
        }
        return std::nullopt;
    }
    if (symbolTable.hasFunction(operandSym->getName())) {
        return ResolvedCallee { symbolTable.findFunction(operandSym->getName()), false };
    }
    return std::nullopt;
}

void SemanticAnalysisVisitor::checkAndConvertCallArgs(
        ast::FunctionCall& functionCall,
        const FunctionEntry& functionSymbol) {
    auto& arguments = functionCall.getArgumentList();
    const bool isVariadic = functionSymbol.getType().isVariadic();
    const bool externalVararg = functionSymbol.getContext() == externalContext();
    const bool arityOk = arguments.size() == functionSymbol.argumentCount()
            || (isVariadic && arguments.size() >= functionSymbol.argumentCount())
            || (externalVararg && arguments.size() >= functionSymbol.argumentCount());
    if (!arityOk) {
        semanticError("no match for function " + functionSymbol.getType().to_string(),
                functionCall.getContext());
        return;
    }

    auto declaredArguments = functionSymbol.arguments();
    const std::size_t checkCount = std::min(arguments.size(), declaredArguments.size());
    for (std::size_t i { 0 }; i < checkCount; ++i) {
        if (!arguments.at(i)->hasResult(store_)) {
            continue;
        }
        const auto* actualArgument = arguments.at(i)->result(store_);
        requireProductAssignable(
                declaredArguments.at(i), actualArgument->getType(), functionCall.getContext());
        maybeSetCallArgConversion(arguments.at(i).get(), declaredArguments.at(i), symbolTable, store_);
    }
    if (isVariadic || externalVararg) {
        for (std::size_t i = declaredArguments.size(); i < arguments.size(); ++i) {
            if (!arguments.at(i)->hasResult(store_)) {
                continue;
            }
            const type::Type& argType = arguments.at(i)->result(store_)->getType();
            // Default argument promotions: float becomes double (printf "%f").
            if (type::isFloating(argType) && argType.getSize() < 8) {
                maybeSetCallArgConversion(arguments.at(i).get(), type::doubleFloating(),
                        symbolTable, store_);
            }
        }
    }
    functionCall.setTypeAndResult(store_, symbolTable.createTemporarySymbol(functionSymbol.returnType()));
}

void SemanticAnalysisVisitor::analyzeBuiltinCall(
        ast::FunctionCall& functionCall,
        const std::string& builtinName,
        const builtins::BuiltinDescriptor& builtin) {
    functionCall.visitArguments(*this);

    std::string symbolName = builtinName.empty() ? "__builtin_va_arg" : builtinName;
    if (builtin.syntheticCallee) {
        symbolName = *builtin.syntheticCallee;
        store_.setCallPlan(&functionCall, symbols::CallPlan::Direct);
    } else if (builtin.builtinPlan) {
        store_.setBuiltinPlan(&functionCall, *builtin.builtinPlan);
    }

    store_.setFunctionSymbol(&functionCall, FunctionEntry {
            symbolName,
            type::function(builtin.returnType, { builtin.syntheticArgType }).getFunction(),
            functionCall.getContext() });

    if (!builtins::builtinArityOk(builtin, functionCall.getArgumentList().size())) {
        semanticError("wrong number of arguments to " + symbolName, functionCall.getContext());
        return;
    }
    if (builtin.returnType.isVoid()) {
        functionCall.setTypeAndResult(store_, symbolTable.createTemporarySymbol(type::signedInteger()));
    } else {
        functionCall.setTypeAndResult(store_, symbolTable.createTemporarySymbol(builtin.returnType));
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    auto* idOperand = dynamic_cast<ast::IdentifierExpression*>(functionCall.getOperandExpression());
    const std::string builtinName = idOperand ? idOperand->getIdentifier() : std::string {};
    const type::Type* vaArgTy = functionCall.builtinTypeArgument();
    if (auto builtin = builtins::lookupBuiltin(builtinName, vaArgTy)) {
        analyzeBuiltinCall(functionCall, builtinName, *builtin);
        return;
    }

    functionCall.visitOperand(*this);
    auto& callArguments = functionCall.getArgumentList();
    for (std::size_t i { 0 }; i < callArguments.size(); ++i) {
        analyzeAsRvalue(*callArguments.at(i));
    }

    if (!functionCall.getOperandExpression()->hasResult(store_)) {
        return;
    }

    auto* operandSym = functionCall.getOperandExpression()->result(store_);
    type::Type operandType = operandSym->getType();

    std::string designatorName;
    std::string sourceCalleeName;
    if (idOperand) {
        sourceCalleeName = idOperand->getIdentifier();
        if (const std::string* dn = idOperand->functionDesignatorName(store_)) {
            designatorName = *dn;
        }
    }

    auto resolved = resolveCallee(designatorName, operandSym, operandType, functionCall.getContext());
    if (!resolved) {
        const std::string display = !sourceCalleeName.empty()
                ? sourceCalleeName
                : operandSym->getName();
        semanticError("called object `" + display + "` is not a function", functionCall.getContext());
        return;
    }

    store_.setFunctionSymbol(&functionCall, resolved->symbol);
    if (resolved->indirect) {
        store_.setCallPlan(&functionCall, symbols::CallPlan::Indirect);
    } else {
        store_.setCallPlan(&functionCall, symbols::CallPlan::Direct);
    }
    checkAndConvertCallArgs(functionCall, resolved->symbol);
}

} // namespace semantic_analyzer
