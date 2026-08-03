#include "SemanticAnalysisVisitorInternal.h"

#include <type_traits>
#include <variant>

namespace semantic_analyzer {

std::optional<SemanticAnalysisVisitor::ResolvedCallee> SemanticAnalysisVisitor::resolveCallee(
        ast::Expression* operandExpr,
        symbols::ValueEntry* operandSym,
        const type::Type& operandType,
        const translation_unit::Context& callContext) {
    if (operandExpr && operandExpr->holdsFunctionDesignator()) {
        const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(
                annotations().addressPlan(operandExpr));
        if (d && d->functionName && symbolTable.hasFunction(*d->functionName)) {
            return ResolvedCallee { symbolTable.findFunction(*d->functionName), false };
        }
    }
    if (operandType.isPointer()) {
        type::Type pointee = operandType.dereference();
        if (pointee.isFunction()) {
            return ResolvedCallee {
                    symbols::FunctionEntry { operandSym->getName(), pointee.getFunction(), callContext },
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
        const symbols::FunctionEntry& functionSymbol) {
    auto& arguments = functionCall.getArgumentList();
    const bool isVariadic = functionSymbol.getType().isVariadic();
    const bool arityOk = arguments.size() == functionSymbol.argumentCount()
            || (isVariadic && arguments.size() >= functionSymbol.argumentCount());
    if (!arityOk) {
        semanticError("no match for function " + functionSymbol.getType().to_string(),
                functionCall.getContext());
        return;
    }

    auto declaredArguments = functionSymbol.arguments();
    const std::size_t checkCount = std::min(arguments.size(), declaredArguments.size());
    for (std::size_t i { 0 }; i < checkCount; ++i) {
        if (!arguments.at(i)->hasResultSymbol(annotations())) {
            continue;
        }
        const auto* actualArgument = arguments.at(i)->getResultSymbol(annotations());
        if (!checkProductAssign(
                declaredArguments.at(i), actualArgument->getType(), functionCall.getContext(),
                arguments.at(i).get())) {
            continue;
        }
        maybeSetConversion(arguments.at(i).get(), declaredArguments.at(i), symbolTable, annotations());
    }
    if (isVariadic) {
        for (std::size_t i = declaredArguments.size(); i < arguments.size(); ++i) {
            if (!arguments.at(i)->hasResultSymbol(annotations())) {
                continue;
            }
            decayArrayInPlace(*arguments.at(i), symbolTable, annotations());
            const type::Type& argType = arguments.at(i)->getResultSymbol(annotations())->getType();
            const type::Type promoted = type::defaultArgPromote(argType);
            if (!promoted.equivalentTo(argType)) {
                maybeSetConversion(arguments.at(i).get(), promoted, symbolTable, annotations());
            }
        }
    }
    functionCall.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(functionSymbol.returnType()));
}

void SemanticAnalysisVisitor::analyzeBuiltinCall(
        ast::FunctionCall& functionCall,
        const std::string& builtinName,
        const builtins::BuiltinDescriptor& builtin) {
    functionCall.visitArguments(*this);

    std::string symbolName = builtinName.empty() ? "__builtin_va_arg" : builtinName;
    type::Type returnType = builtin.returnType;
    bool ok = true;
    std::visit([&](const auto& kind) {
        using K = std::decay_t<decltype(kind)>;
        if constexpr (std::is_same_v<K, builtins::TypeNameReturn>) {
            annotations().setBuiltinPlan(&functionCall, symbols::VaArgPlan {});
            if (!functionCall.builtinTypeName()) {
                semanticError("malformed __builtin_va_arg", functionCall.getContext());
                ok = false;
                return;
            }
            const translation_unit::Context callCtx = functionCall.getContext();
            auto parsed = resolveTypeName(
                    *functionCall.builtinTypeName(), *this,
                    [this](std::string msg, const translation_unit::Context& ctx) {
                        semanticError(std::move(msg), ctx);
                    },
                    &callCtx);
            if (!parsed) {
                ok = false;
                return;
            }
            returnType = *parsed;
        } else if constexpr (std::is_same_v<K, builtins::SimpleBuiltin>) {
            annotations().setBuiltinPlan(&functionCall, kind.plan);
        }
    }, builtin.kind);
    if (!ok) {
        return;
    }

    annotations().setFunctionSymbol(&functionCall, symbols::FunctionEntry {
            symbolName,
            type::function(builtin.returnType, { builtin.argType }).getFunction(),
            functionCall.getContext() });

    if (!builtins::builtinArityOk(builtin, functionCall.getArgumentList().size())) {
        semanticError("wrong number of arguments to " + symbolName, functionCall.getContext());
        return;
    }
    if (returnType.isVoid()) {
        functionCall.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
    } else {
        functionCall.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(returnType));
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    auto* idOperand = dynamic_cast<ast::IdentifierExpression*>(functionCall.getOperandExpression());
    const std::string builtinName = idOperand ? idOperand->getIdentifier() : std::string {};
    if (gnuExtensions_) {
        if (functionCall.isGnuConstantP()) {
            type::IntegerConstant folded;
            if (functionCall.evaluateConstant(folded)) {
                functionCall.setTypeAndResult(annotations(),
                        symbolTable.createTemporarySymbol(type::signedInteger()));
                return;
            }
            semanticError("wrong number of arguments to __builtin_constant_p",
                    functionCall.getContext());
            return;
        }
        if (auto builtin = builtins::lookupBuiltin(builtinName)) {
            analyzeBuiltinCall(functionCall, builtinName, *builtin);
            return;
        }
    }

    functionCall.visitOperand(*this);
    auto& callArguments = functionCall.getArgumentList();
    for (std::size_t i { 0 }; i < callArguments.size(); ++i) {
        analyzeAsRvalue(*callArguments.at(i));
    }

    if (!functionCall.getOperandExpression()->hasResultSymbol(annotations())) {
        return;
    }

    auto* operandSym = functionCall.getOperandExpression()->getResultSymbol(annotations());
    type::Type operandType = operandSym->getType();

    auto resolved = resolveCallee(functionCall.getOperandExpression(), operandSym, operandType,
            functionCall.getContext());
    if (!resolved) {
        const std::string display = idOperand ? idOperand->getIdentifier() : operandSym->getName();
        semanticError("called object `" + display + "` is not a function", functionCall.getContext());
        return;
    }

    annotations().setFunctionSymbol(&functionCall, resolved->symbol);
    if (resolved->indirect) {
        annotations().setCallPlan(&functionCall, symbols::CallPlan::Indirect);
    } else {
        annotations().setCallPlan(&functionCall, symbols::CallPlan::Direct);
    }
    checkAndConvertCallArgs(functionCall, resolved->symbol);
}

} // namespace semantic_analyzer
