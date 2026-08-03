#ifndef SYMBOLS_CALLPLAN_H_
#define SYMBOLS_CALLPLAN_H_

// SA→CG call shape. Callee identity is not stored here:
//   Direct   → FunctionEntry name on the call node (functionSymbol)
//   Indirect → operand Result value holding the address
// Product builtins / va_* use BuiltinPlan, not CallPlan.

namespace symbols {

enum class CallPlan {
    Direct,
    Indirect,
};

inline bool isIndirectCall(CallPlan plan) {
    return plan == CallPlan::Indirect;
}

inline bool isDirectCall(CallPlan plan) {
    return plan == CallPlan::Direct;
}

} // namespace symbols

#endif // SYMBOLS_CALLPLAN_H_
