#include "SemanticAnalysisVisitorInternal.h"

#include <cassert>
#include <optional>

#include "ast/GnuBuiltinFunctions.h"
#include "ast/IdentifierExpression.h"

namespace semantic_analyzer {

namespace {

// Precondition: symbolTable.hasFunction(identifier.getIdentifier()).
void setFunctionDesignator(ast::IdentifierExpression& identifier, SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    const std::string& name = identifier.getIdentifier();
    assert(symbolTable.hasFunction(name));
    auto functionEntry = symbolTable.findFunction(name);
    type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
    auto addr = symbolTable.createTemporarySymbol(type::pointer(fnType));
    identifier.setFunctionDesignatorResult(store, addr, fnType);
    symbols::FunctionDesignatorPlan plan;
    plan.functionName = functionEntry.getName();
    store.setAddressPlan(&identifier, symbols::AddressPlan { plan });
    assert(identifier.holdsFunctionDesignator());
    assert(symbols::get_if<symbols::FunctionDesignatorPlan>(store.addressPlan(&identifier)));
}

// Resolved call target: type for arity/return; CallPlan shape for codegen.
struct Callee {
    symbols::CallPlan plan;
    type::Function type;
};

std::optional<Callee> resolveCallee(ast::FunctionCall& functionCall, SymbolTable& symbolTable,
        symbols::AnnotationStore& store, std::string& errorDisplay) {
    auto* operandSym = functionCall.operandSymbol(store);
    type::Type operandType = operandSym->getType();
    auto* operandExpr = functionCall.getOperandExpression();

    // Designator label lives on FunctionDesignatorPlan; address temp is Result.
    // Form is the cheap tag.
    if (operandExpr->holdsFunctionDesignator()) {
        const auto* addrPlan = store.addressPlan(operandExpr);
        const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(addrPlan);
        assert(d && "designator form without FunctionDesignatorPlan on the store");
        if (d && d->functionName) {
            auto entry = symbolTable.findFunction(*d->functionName);
            return Callee {
                symbols::DirectCallPlan { *d->functionName },
                entry.getType(),
            };
        }
    }

    if (type::isPointerToBareFunction(operandType)) {
        type::Type pointee = operandType.dereference();
        return Callee {
            symbols::IndirectCallPlan { operandSym->getName() },
            pointee.getFunction(),
        };
    }

    if (auto* id = dynamic_cast<ast::IdentifierExpression*>(operandExpr)) {
        errorDisplay = id->getIdentifier();
    } else {
        errorDisplay = unscopedSymbolName(operandSym->getName());
    }
    return std::nullopt;
}

enum class VaBuiltinKind { Start, Arg, End, Copy };

struct VaBuiltinSpec {
    const char* name;
    VaBuiltinKind kind;
    std::size_t minArgs;
    std::size_t maxArgs;
};

constexpr VaBuiltinSpec kVaBuiltins[] = {
        { "__builtin_va_start", VaBuiltinKind::Start, 1, 2 },
        { "__builtin_c23_va_start", VaBuiltinKind::Start, 1, 2 },
        { "__builtin_va_end", VaBuiltinKind::End, 1, 1 },
        { "__builtin_va_copy", VaBuiltinKind::Copy, 2, 2 },
        { "__builtin_va_arg", VaBuiltinKind::Arg, 1, 1 },
};

const VaBuiltinSpec* lookupVaBuiltin(const std::string& name) {
    for (const auto& spec : kVaBuiltins) {
        if (name == spec.name) {
            return &spec;
        }
    }
    return nullptr;
}

bool analyzeVaBuiltin(ast::FunctionCall& functionCall, const VaBuiltinSpec& spec,
        SymbolTable& symbolTable, symbols::AnnotationStore& store,
        SemanticAnalysisVisitor& visitor) {
    functionCall.visitArguments(visitor);

    if (spec.kind == VaBuiltinKind::Arg && !functionCall.builtinTypeArgument()) {
        visitor.semanticError("wrong number of arguments to " + std::string { spec.name },
                functionCall.getContext());
        return true;
    }

    const auto& args = functionCall.getArgumentList();
    if (args.size() < spec.minArgs || args.size() > spec.maxArgs) {
        visitor.semanticError("wrong number of arguments to " + std::string { spec.name },
                functionCall.getContext());
        return true;
    }

    symbols::CallPlan plan;
    switch (spec.kind) {
    case VaBuiltinKind::Start:
        plan = symbols::VaStartPlan {};
        break;
    case VaBuiltinKind::End:
        plan = symbols::VaEndPlan {};
        break;
    case VaBuiltinKind::Copy:
        plan = symbols::VaCopyPlan {};
        break;
    case VaBuiltinKind::Arg:
        plan = symbols::VaArgPlan {};
        break;
    }
    store.setCallPlan(&functionCall, std::move(plan));
    if (spec.kind == VaBuiltinKind::Arg) {
        functionCall.setResultSymbol(store,
                symbolTable.createTemporarySymbol(*functionCall.builtinTypeArgument()));
    }
    return true;
}

void analyzeConstantP(ast::FunctionCall& functionCall, SymbolTable& symbolTable,
        symbols::AnnotationStore& store, SemanticAnalysisVisitor& visitor) {
    functionCall.visitArguments(visitor);
    if (functionCall.getArgumentList().size() != 1) {
        visitor.semanticError("wrong number of arguments to __builtin_constant_p",
                functionCall.getContext());
        return;
    }
    functionCall.setResultSymbol(store, symbolTable.createTemporarySymbol(type::signedInteger()));
}

} // namespace

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    auto* idOperand = dynamic_cast<ast::IdentifierExpression*>(functionCall.getOperandExpression());
    if (gnuExtensions_ && idOperand) {
        if (const VaBuiltinSpec* spec = lookupVaBuiltin(idOperand->getIdentifier())) {
            analyzeVaBuiltin(functionCall, *spec, symbolTable, annotations(), *this);
            return;
        }
        if (ast::isGnuConstantPBuiltin(idOperand->getIdentifier())) {
            analyzeConstantP(functionCall, symbolTable, annotations(), *this);
            return;
        }
    }

    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    if (!functionCall.hasOperandSymbol(annotations())) {
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
        if (argument->hasResultSymbol(annotations())) {
            rejectFunctionValue(argument->getResultSymbol(annotations())->getType(), functionCall.getContext());
        }
    }

    const auto declaredArguments = callee.type.getArguments();
    const bool variadic = callee.type.isVariadic();

    if (arguments.size() < declaredArguments.size()
            || (arguments.size() > declaredArguments.size() && !variadic)) {
        semanticError("no match for function " + type::function(callee.type.getReturnType(),
                declaredArguments, callee.type.isVariadic()).to_string(), functionCall.getContext());
        return;
    }

    for (std::size_t i { 0 }; i < declaredArguments.size(); ++i) {
        if (!arguments.at(i)->hasResultSymbol(annotations())) {
            return;
        }
        type::Type actual = arguments.at(i)->getResultSymbol(annotations())->getType();
        if (actual.isArray()) {
            actual = actual.decayArray();
        }
        checkAssign(declaredArguments.at(i), actual, functionCall.getContext(), arguments.at(i).get());
        decayArrayToPointer(*arguments.at(i), declaredArguments.at(i), symbolTable, annotations());
        if (!arguments.at(i)->holdsAggregateAddress()) {
            maybeSetConversion(arguments.at(i).get(), declaredArguments.at(i),
                    symbolTable, annotations());
        }
    }
    for (std::size_t i = declaredArguments.size(); i < arguments.size(); ++i) {
        if (!arguments.at(i)->hasResultSymbol(annotations())) {
            continue;
        }
        decayArrayValue(*arguments.at(i), symbolTable, annotations());
        const type::Type& argType = arguments.at(i)->getResultSymbol(annotations())->getType();
        const type::Type promoted = type::defaultArgPromote(argType);
        if (!promoted.equivalentTo(argType)) {
            annotations().setConversion(arguments.at(i).get(),
                    symbolTable.createTemporarySymbol(promoted));
        }
    }

    annotations().setCallPlan(&functionCall, callee.plan);

    auto returnType = callee.type.getReturnType();
    if (!returnType.isVoid()) {
        functionCall.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(returnType));
    }
}

void SemanticAnalysisVisitor::visit(ast::IdentifierExpression& identifier) {
    const std::string& name = identifier.getIdentifier();

    // Ordinary objects/functions hide enumerators in the same scope (C).
    // Prefer a visible symbol before folding a TU-level enumerator.
    // Clear parse-time enum fold (CSNB) so the name is an lvalue again.
    if (symbolTable.hasSymbol(name)) {
        identifier.clearFoldedConstant();
        auto entry = symbolTable.lookup(name);
        if (type::isBareFunction(entry.getType())) {
            // Dual table (see SymbolTable::insertFunction): bare-function ValueEntry for
            // visibility plus functions[] for designator metadata. Parameters are
            // pointer-to-function after adjustedParameterType, so they take the value path.
            if (!symbolTable.hasFunction(name)) {
                semanticError("symbol `" + name + "` is not a function", identifier.getContext());
                return;
            }
            setFunctionDesignator(identifier, symbolTable, annotations());
            return;
        }
        identifier.setResultSymbol(annotations(), entry);
        return;
    }

    if (symbolTable.hasEnumConstant(name)) {
        type::IntegerConstant ice = symbolTable.getEnumConstant(name);
        identifier.setFoldedConstant(ice);
        identifier.setTypeAndResult(annotations(),
                symbolTable.createTemporarySymbol(ice.type));
        return;
    }

    if (name == "__func__" || name == "__FUNCTION__" || name == "__PRETTY_FUNCTION__") {
        if (currentFunctionName.empty()) {
            semanticError("__func__ used outside a function", identifier.getContext());
            return;
        }
        const std::string literal = "\"" + currentFunctionName + "\"";
        identifier.setStringConstantLabel(symbolTable.newConstant(literal));
        identifier.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(
                type::pointer(type::signedCharacter(), { type::Qualifier::CONST })));
        return;
    }

    semanticError("symbol `" + name + "` is not defined", identifier.getContext());
}

} // namespace semantic_analyzer
