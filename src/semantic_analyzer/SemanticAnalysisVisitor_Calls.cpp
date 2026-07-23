#include "SemanticAnalysisVisitorInternal.h"
#include "Symbols.h"

namespace semantic_analyzer {




void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    // Expression-form GCC builtins: recognize before visiting the designator so
    // undeclared __builtin_* names are not hard errors.
    std::string builtinName;
    if (auto* id = dynamic_cast<ast::IdentifierExpression*>(functionCall.getOperandExpression())) {
        builtinName = id->getIdentifier();
    }
    auto builtin = ast::lookupBuiltin(functionCall.getBuiltinKind(), builtinName,
            functionCall.getVaArgResultType());
    if (builtin) {
        functionCall.visitArguments(*this);
        functionCall.setBuiltinKind(builtin->kind);
        functionCall.setIndirect(false);
        const std::string syntheticName = builtin->syntheticName
                ? builtin->syntheticName
                : (builtinName.empty() ? "__builtin_va_arg" : builtinName);
        functionCall.setSymbol(symbols::FunctionEntry {
                syntheticName,
                type::function(builtin->returnType, { builtin->syntheticArgType }).getFunction(),
                functionCall.getContext() });

        if (!ast::builtinArityOk(*builtin, functionCall.getArgumentList().size())) {
            semanticError("wrong number of arguments to " + syntheticName, functionCall.getContext());
            return;
        }
        // void builtins still need a placeholder result for expression context.
        if (builtin->returnType.isVoid()) {
            functionCall.setTypedResult(symbolTable.createTemporarySymbol(type::signedInteger()));
        } else {
            functionCall.setTypedResult(symbolTable.createTemporarySymbol(builtin->returnType));
        }
        return;
    }

    functionCall.visitOperand(*this);
    // Argument value context: visit + array-to-pointer decay (C 6.3.2.1).
    auto& callArguments = functionCall.getArgumentList();
    for (std::size_t i { 0 }; i < callArguments.size(); ++i) {
        analyzeAsRvalue(*callArguments.at(i));
    }

    // Operand failed to resolve (e.g. undeclared identifier) — error already reported.
    if (!functionCall.hasOperandSymbol()) {
        return;
    }

    auto* operandSym = functionCall.operandSymbol();
    type::Type operandType = operandSym->getType();

    // Named designator (possibly after decay to a pointer temporary) -> direct call.
    // Only use the source identifier when the operand is a true function designator.
    // Do not fall back to hasFunction(id) when a local shadows a same-named function —
    // that would call the function through a non-callable local (fuzz regression).
    std::string designatorName;
    std::string sourceCalleeName;
    if (auto* id = dynamic_cast<ast::IdentifierExpression*>(functionCall.getOperandExpression())) {
        sourceCalleeName = id->getIdentifier();
        if (id->hasFunctionDesignator()) {
            designatorName = id->getFunctionDesignator();
        }
    }

    symbols::FunctionEntry functionSymbol { "", type::function(type::voidType()).getFunction(), externalContext() };
    bool resolved = false;

    if (!designatorName.empty() && symbolTable.hasFunction(designatorName)) {
        functionSymbol = symbolTable.findFunction(designatorName);
        functionCall.setIndirect(false);
        resolved = true;
    } else if (operandType.isPointer()) {
        type::Type pointee = operandType.dereference();
        if (pointee.isFunction()) {
            // Call through function pointer value.
            functionSymbol = symbols::FunctionEntry {
                    operandSym->getName(), pointee.getFunction(), functionCall.getContext() };
            functionCall.setIndirect(true);
            resolved = true;
        }
    } else if (operandType.isFunction() && symbolTable.hasFunction(operandSym->getName())) {
        functionSymbol = symbolTable.findFunction(operandSym->getName());
        functionCall.setIndirect(false);
        resolved = true;
    } else if (symbolTable.hasFunction(operandSym->getName())) {
        functionSymbol = symbolTable.findFunction(operandSym->getName());
        functionCall.setIndirect(false);
        resolved = true;
    }

    if (!resolved) {
        // Prefer the source identifier in diagnostics (not a scope-mangled local name).
        const std::string display = !sourceCalleeName.empty()
                ? sourceCalleeName
                : (operandSym ? operandSym->getName() : std::string{"<unknown>"});
        semanticError("called object `" + display + "` is not a function", functionCall.getContext());
        return;
    }

    functionCall.setSymbol(functionSymbol);

    auto& arguments = functionCall.getArgumentList();
    const bool isVariadic = functionSymbol.getType().isVariadic();
    const bool externalVararg = functionSymbol.getContext() == externalContext();
    if (arguments.size() == functionSymbol.argumentCount()
            || (isVariadic && arguments.size() >= functionSymbol.argumentCount())
            || (externalVararg && arguments.size() >= functionSymbol.argumentCount())) {
        auto declaredArguments = functionSymbol.arguments();
        const std::size_t checkCount = std::min(arguments.size(), declaredArguments.size());
        for (std::size_t i { 0 }; i < checkCount; ++i) {
            const auto& declaredArgument = declaredArguments.at(i);
            // Argument visit already reported errors (e.g. undeclared) and left no
            // result symbol — skip assign check rather than null-deref (fuzz/UBSAN).
            if (!arguments.at(i)->hasResultSymbol()) {
                continue;
            }
            const auto* actualArgument = arguments.at(i)->getResultSymbol();
            requireProductAssignable(actualArgument->getType(), declaredArgument, functionCall.getContext());
        }

        // Implicit float<->int conversion for formals (C 6.5.2.2). Assignment and
        // return already emit cvttsd2si via Assign; call args must too. Without
        // this, record_rename_pair(..., MAX_SCORE) with MAX_SCORE=60000.0 stores
        // score 0 (git empty-file rename: "similarity index 0%").
        // Keep arg result as the source type so codegen writes the float there;
        // conversion target is a separate temp of the formal type (like return).
        for (std::size_t i { 0 }; i < checkCount; ++i) {
            if (!arguments.at(i)->hasResultSymbol()) {
                continue;
            }
            maybeSetCallArgConversion(arguments.at(i).get(), declaredArguments.at(i), symbolTable);
        }

        auto returnType = functionSymbol.returnType();
        // Always set a result type so void calls can appear in expression context
        // (e.g. assert macros: cond ? (void)0 : __assert_fail(...)).
        functionCall.setTypedResult(symbolTable.createTemporarySymbol(returnType));
    } else {
        semanticError("no match for function " + functionSymbol.getType().to_string(), functionCall.getContext());
    }
}

} // namespace semantic_analyzer
