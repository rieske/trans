#include "CodeGeneratingVisitor.h"
#include "ast/AstNodes.h"

#include <type_traits>
#include <variant>

#include "ast/GnuBuiltinFunctions.h"
#include "codegen/IrBuilders.h"
#include "types/IntegerConstant.h"
#include "types/ObjectAbiType.h"

namespace codegen {

bool CodeGeneratingVisitor::tryEmitGnuDirectCall(ast::FunctionCall& functionCall,
        const std::string& calleeName) {
    const auto* bswap = ast::findGnuBswapBuiltin(calleeName);
    const auto* ctz = ast::findGnuCtzBuiltin(calleeName);
    if (!bswap && !ctz && !ast::isGnuAllocaBuiltin(calleeName)) {
        return false;
    }
    functionCall.visitArguments(*this);
    const auto& args = functionCall.getArgumentList();
    const int arg = convertedResult(*args[0]);
    const int result = id(*functionCall.getResultSymbol(store_));
    if (bswap) {
        emit(ir::bswap(arg, result, bswap->widthBytes));
    } else if (ctz) {
        emit(ir::ctz(arg, result, ctz->widthBytes));
    } else {
        emit(ir::allocaBytes(arg, result));
    }
    return true;
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    type::IntegerConstant folded;
    if (functionCall.evaluateConstant(folded) && functionCall.hasResultSymbol(store_)) {
        emitIntegerConstant(folded, id(*functionCall.getResultSymbol(store_)));
        return;
    }

    const symbols::CallPlan* plan = store_.callPlan(&functionCall);
    if (!plan) {
        functionCall.visitOperand(*this);
        functionCall.visitArguments(*this);
        return;
    }

    if (const auto* direct = symbols::get_if<symbols::DirectCallPlan>(plan)) {
        if (tryEmitGnuDirectCall(functionCall, direct->calleeName)) {
            return;
        }
    }

    std::visit(
            [&](const auto& arm) {
                using T = std::decay_t<decltype(arm)>;
                if constexpr (std::is_same_v<T, symbols::VaStartPlan>
                        || std::is_same_v<T, symbols::VaEndPlan>
                        || std::is_same_v<T, symbols::VaCopyPlan>
                        || std::is_same_v<T, symbols::VaArgPlan>) {
                    functionCall.visitArguments(*this);
                    const auto& args = functionCall.getArgumentList();
                    if constexpr (std::is_same_v<T, symbols::VaStartPlan>) {
                        int lastStorage = kNoSymbol;
                        if (args.size() >= 2) {
                            lastStorage = id(*args[1]->getResultSymbol(store_));
                        }
                        emit(ir::vaStart(id(*args[0]->getResultSymbol(store_)), lastStorage));
                    } else if constexpr (std::is_same_v<T, symbols::VaEndPlan>) {
                        emit(ir::vaEnd());
                    } else if constexpr (std::is_same_v<T, symbols::VaCopyPlan>) {
                        emit(ir::vaCopy(id(*args[0]->getResultSymbol(store_)),
                                id(*args[1]->getResultSymbol(store_))));
                    } else {
                        emit(ir::vaArg(id(*args[0]->getResultSymbol(store_)),
                                id(*functionCall.getResultSymbol(store_))));
                    }
                } else {
                    functionCall.visitOperand(*this);
                    functionCall.visitArguments(*this);
                    for (auto& expression : functionCall.getArgumentList()) {
                        emit(ir::argument(convertedResult(*expression)));
                    }
                    int memoryReturnDest = kNoSymbol;
                    if (functionCall.hasResultSymbol(store_) && !functionCall.expressionType().isVoid()) {
                        if (type::object_abi::typeNeedsMemoryReturn(functionCall.expressionType())) {
                            memoryReturnDest = id(*functionCall.getResultSymbol(store_));
                        }
                    }
                    emit(ir::call(id(symbols::callCalleeName(*plan)), symbols::isIndirectCall(*plan),
                            memoryReturnDest));
                    if (functionCall.hasResultSymbol(store_) && !functionCall.expressionType().isVoid()) {
                        emit(ir::retrieve(id(*functionCall.getResultSymbol(store_)),
                                memoryReturnDest >= 0));
                    }
                    if (const auto* direct = symbols::get_if<symbols::DirectCallPlan>(plan);
                            direct && direct->noreturn) {
                        emit(ir::voidReturn());
                    }
                }
            },
            *plan);
}

} // namespace codegen
