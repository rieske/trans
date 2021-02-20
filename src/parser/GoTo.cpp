#include "GoTo.h"

namespace parser {

GoTo::GoTo(const Closure& closure) :
    closure { closure }
{
}

GoTo::~GoTo() {
}

std::vector<LR1Item> GoTo::operator()(const std::vector<LR1Item>& I, const GrammarSymbol& X) const {
    std::vector<LR1Item> goto_I_X;
    for (const auto& existingItem : I) {
        if (existingItem.hasUnvisitedSymbols() && (existingItem.nextUnvisitedSymbol() == X)) { // [ A -> a.Xb, c ]
            goto_I_X.push_back(existingItem.advance());
        }
    }
    closure(goto_I_X);
    return goto_I_X;
}

} // namespace parser

