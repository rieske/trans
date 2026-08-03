#ifndef SYMBOLS_BUILTINPLAN_H_
#define SYMBOLS_BUILTINPLAN_H_

// SA→CG product builtin lowering (not a CallPlan arm).
// Ordinary calls use CallPlan; __builtin_alloca is AllocaPlan (rsp subtract).

#include <variant>

namespace symbols {

struct VaStartPlan {};
struct VaEndPlan {};
struct VaCopyPlan {};
struct VaArgPlan {};
struct AllocaPlan {};
struct CtzPlan {
    int widthBytes { 8 };
};
struct BswapPlan {
    int widthBytes { 0 };
};

using BuiltinPlan = std::variant<
        VaStartPlan,
        VaEndPlan,
        VaCopyPlan,
        VaArgPlan,
        AllocaPlan,
        CtzPlan,
        BswapPlan>;

} // namespace symbols

#endif // SYMBOLS_BUILTINPLAN_H_
