#ifndef SYMBOLS_POINTERARITHPLAN_H_
#define SYMBOLS_POINTERARITHPLAN_H_

#include <string>
#include <variant>

namespace symbols {

struct PointerScalePlan {
    int scale;
    std::string scaleTempName;
    bool pointerOnLeft; // binary +/- ; compound assign always scales RHS
};

struct PointerDifferencePlan {
    int scale;
    std::string scaleTempName;
};

using PointerArithPlan = std::variant<PointerScalePlan, PointerDifferencePlan>;

} // namespace symbols

#endif // SYMBOLS_POINTERARITHPLAN_H_
