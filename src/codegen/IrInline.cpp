#include "IrInline.h"

#include "SymbolRefs.h"

namespace codegen {
namespace {

bool isVaOp(Op op) {
    return op == Op::VaStart || op == Op::VaArg || op == Op::VaCopy || op == Op::VaEnd;
}

} // namespace

bool isSetjmpFamily(std::string_view name) {
    return name == "setjmp" || name == "_setjmp" || name == "sigsetjmp" || name == "__sigsetjmp"
            || name == "longjmp" || name == "_longjmp" || name == "siglongjmp"
            || name == "__longjmp_chk";
}

int nonLabelCount(const std::vector<Instruction>& body) {
    int n = 0;
    for (const auto& inst : body) {
        if (inst.op != Op::Label) {
            ++n;
        }
    }
    return n;
}

bool calleeLooksUnsafe(const Procedure& callee, const IrStringTable& strings) {
    for (const auto& inst : callee.body) {
        if (isVaOp(inst.op)) {
            return true;
        }
        if (inst.op == Op::Call && !inst.callIndirect && isSetjmpFamily(strings.get(inst.arg0))) {
            return true;
        }
    }
    return false;
}

bool callIsEligible(const Instruction& call,
        const Procedure& caller,
        const Procedure& callee,
        const IrStringTable& strings,
        const InlineCaps& caps,
        int paramCount,
        int inlinesOnCaller,
        int inlinesInTu,
        bool calleeFinished) {
    if (call.op != Op::Call || call.callIndirect) {
        return false;
    }
    if (call.arg0 != callee.name) {
        return false;
    }
    if (callee.variadic || calleeLooksUnsafe(callee, strings)) {
        return false;
    }
    if (!calleeFinished || callee.name == caller.name) {
        return false;
    }
    if (nonLabelCount(callee.body) > caps.maxCalleeInsts) {
        return false;
    }
    if (nonLabelCount(caller.body) + nonLabelCount(callee.body) > caps.maxCallerInsts) {
        return false;
    }
    if (inlinesOnCaller >= caps.maxInlinesPerCaller || inlinesInTu >= caps.maxTotalInlines) {
        return false;
    }
    if (paramCount != static_cast<int>(callee.frame.arguments.size())) {
        return false;
    }
    if (callee.memoryReturn != (call.memoryReturnDest != kNoSymbol)) {
        return false;
    }
    return true;
}

} // namespace codegen
