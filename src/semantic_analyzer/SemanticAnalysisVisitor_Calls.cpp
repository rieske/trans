#include "SemanticAnalysisVisitorInternal.h"

#include <type_traits>
#include <variant>

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
        if (!arguments.at(i)->hasResult(annotations())) {
            continue;
        }
        const auto* actualArgument = arguments.at(i)->result(annotations());
        requireProductAssignable(
                declaredArguments.at(i), actualArgument->getType(), functionCall.getContext());
        maybeSetConversion(arguments.at(i).get(), declaredArguments.at(i), symbolTable, annotations());
    }
    if (isVariadic) {
        for (std::size_t i = declaredArguments.size(); i < arguments.size(); ++i) {
            if (!arguments.at(i)->hasResult(annotations())) {
                continue;
            }
            const type::Type& argType = arguments.at(i)->result(annotations())->getType();
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
        if constexpr (std::is_same_v<K, builtins::SyntheticCall>) {
            symbolName = kind.callee;
            annotations().setCallPlan(&functionCall, symbols::CallPlan::Direct);
        } else if constexpr (std::is_same_v<K, builtins::TypeNameReturn>) {
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

    annotations().setFunctionSymbol(&functionCall, FunctionEntry {
            symbolName,
            type::function(builtin.returnType, { builtin.syntheticArgType }).getFunction(),
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

    if (!functionCall.getOperandExpression()->hasResult(annotations())) {
        return;
    }

    auto* operandSym = functionCall.getOperandExpression()->result(annotations());
    type::Type operandType = operandSym->getType();

    std::string designatorName;
    std::string sourceCalleeName;
    if (idOperand) {
        sourceCalleeName = idOperand->getIdentifier();
        if (const std::string* dn = idOperand->functionDesignatorName(annotations())) {
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

    annotations().setFunctionSymbol(&functionCall, resolved->symbol);
    if (resolved->indirect) {
        annotations().setCallPlan(&functionCall, symbols::CallPlan::Indirect);
    } else {
        annotations().setCallPlan(&functionCall, symbols::CallPlan::Direct);
    }
    checkAndConvertCallArgs(functionCall, resolved->symbol);
}

} // namespace semantic_analyzer
