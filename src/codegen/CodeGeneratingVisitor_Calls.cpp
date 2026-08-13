#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"
#include "Instruction.h"

#include "ast/FunctionCall.h"
#include "types/ObjectAbiType.h"
#include "types/TypeQuery.h"
#include "util/ProductApprox.h"

#include <stdexcept>
#include <type_traits>
#include <variant>

namespace codegen {

namespace {

[[noreturn]] void requireArgCount(const char* what, std::size_t have, std::size_t minNeed) {
    throw std::runtime_error {
            std::string { "FunctionCall " } + what + " needs >= " + std::to_string(minNeed)
                    + " args, have " + std::to_string(have) };
}

// After accept, materialize field load / array decay / conversion for one arg.
// Must not run generateExpression here during the PARAM phase: nested calls must
// finish their own CALL before the outer function queues PARAM.
std::string materializeCallArg(CodeGeneratingVisitor& self, ast::Expression& arg,
        symbols::AnnotationStore& store) {
    materializeFieldIndexLoad(arg, self, store);
    materializeArrayDecay(arg, self, store);
    return self.materializeConversion(arg);
}

// Evaluate every argument (nested calls fully lower) before any PARAM, then
// materialize and queue PARAMs. Interleaving generateExpression+argument per
// arg mis-orders nested CALL vs outer PARAM on the StackMachine queue.
void emitRealCall(CodeGeneratingVisitor& self, ast::FunctionCall& functionCall,
        const symbols::CallPlan& plan, symbols::AnnotationStore& store) {
    // Operand may be an undeclared builtin designator rewritten to a Direct call
    // (e.g. __builtin_alloca → malloc) with no SA result on the identifier.
    if (functionCall.getOperandExpression()->hasResult(store)) {
        self.generateExpression(*functionCall.getOperandExpression());
    }
    const auto& argumentList = functionCall.getArgumentList();
    for (std::size_t i { 0 }; i < argumentList.size(); ++i) {
        argumentList.at(i)->accept(self);
    }
    for (std::size_t i { 0 }; i < argumentList.size(); ++i) {
        self.emit(ir::argument(materializeCallArg(self, *argumentList.at(i), store)));
    }

    auto* fn = store.functionSymbol(&functionCall);
    if (!fn) {
        throw std::runtime_error { "FunctionCall missing FunctionEntry from SA" };
    }

    std::string memoryReturnDest;
    if (!fn->returnType().isVoid()) {
        if (type::object_abi::typeNeedsMemoryReturn(fn->returnType())) {
            memoryReturnDest = functionCall.result(store)->getName();
        }
    }

    const bool indirect = symbols::isIndirectCall(plan);
    std::string calleeName;
    if (indirect) {
        calleeName = functionCall.getOperandExpression()->result(store)->getName();
    } else {
        calleeName = fn->getName();
    }

    self.emit(ir::call(calleeName, indirect, memoryReturnDest));
    if (!fn->returnType().isVoid()) {
        self.emit(ir::retrieve(
                functionCall.result(store)->getName(), !memoryReturnDest.empty()));
    }
}

void emitBuiltinPlan(CodeGeneratingVisitor& self, ast::FunctionCall& functionCall,
        const symbols::BuiltinPlan& plan, symbols::AnnotationStore& store) {
    const auto& args = functionCall.getArgumentList();
    const std::string result = functionCall.result(store)->getName();

    std::visit(
            [&](const auto& arm) {
                using T = std::decay_t<decltype(arm)>;
                if constexpr (std::is_same_v<T, symbols::ConstantZeroPlan>) {
                    self.emit(ir::assignConstant(
                            std::to_string(product_approx::foldConstantP()), result));
                } else if constexpr (std::is_same_v<T, symbols::VaStartPlan>) {
                    if (args.empty()) {
                        requireArgCount("VaStart", args.size(), 1);
                    }
                    self.generateExpression(*args[0]);
                    self.emit(ir::vaStart(args[0]->result(store)->getName()));
                } else if constexpr (std::is_same_v<T, symbols::VaEndPlan>) {
                } else if constexpr (std::is_same_v<T, symbols::VaCopyPlan>) {
                    if (args.size() < 2) {
                        requireArgCount("VaCopy", args.size(), 2);
                    }
                    self.generateExpression(*args[0]);
                    self.generateExpression(*args[1]);
                    self.emit(ir::vaCopy(args[0]->result(store)->getName(),
                            args[1]->result(store)->getName()));
                } else if constexpr (std::is_same_v<T, symbols::VaArgPlan>) {
                    if (args.empty()) {
                        requireArgCount("VaArg", args.size(), 1);
                    }
                    self.generateExpression(*args[0]);
                    self.emit(ir::vaArg(args[0]->result(store)->getName(), result));
                } else if constexpr (std::is_same_v<T, symbols::CtzPlan>) {
                    if (args.empty()) {
                        requireArgCount("Ctz", args.size(), 1);
                    }
                    self.emit(ir::ctz(self.generateExpression(*args[0]), result));
                } else if constexpr (std::is_same_v<T, symbols::BswapPlan>) {
                    if (args.empty()) {
                        requireArgCount("Bswap", args.size(), 1);
                    }
                    self.emit(ir::bswap(self.generateExpression(*args[0]), result, arm.widthBytes));
                }
            },
            plan);
}

} // namespace

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    if (const symbols::BuiltinPlan* bp = store_.builtinPlan(&functionCall)) {
        emitBuiltinPlan(*this, functionCall, *bp, store_);
        return;
    }

    const symbols::CallPlan* planPtr = store_.callPlan(&functionCall);
    if (!planPtr) {
        if (!functionCall.hasResult(store_)) {
            return;
        }
        throw std::runtime_error { "FunctionCall missing CallPlan from SA" };
    }
    emitRealCall(*this, functionCall, *planPtr, store_);
}

} // namespace codegen
