#ifndef SYMBOLS_BUILTINPLAN_H_
#define SYMBOLS_BUILTINPLAN_H_

// SA→CG product builtin lowering (not a CallPlan arm).
// Ordinary calls use CallPlan; __builtin_alloca is rewritten to CallPlan::Direct + FunctionEntry name "malloc".

#include <variant>

namespace symbols {

struct ConstantZeroPlan {};
struct VaStartPlan {};
struct VaEndPlan {};
struct VaCopyPlan {};
struct VaArgPlan {};
struct CtzPlan {};
struct BswapPlan {
    int widthBytes { 0 };
};

using BuiltinPlan = std::variant<
        ConstantZeroPlan,
        VaStartPlan,
        VaEndPlan,
        VaCopyPlan,
        VaArgPlan,
        CtzPlan,
        BswapPlan>;

} // namespace symbols

#endif // SYMBOLS_BUILTINPLAN_H_
