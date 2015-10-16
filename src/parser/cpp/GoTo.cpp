#include "parser/GoTo.h"

using std::vector;

namespace parser {

GoTo::GoTo(const Closure& closure) :
        closure { closure }
{
}

GoTo::~GoTo() {
}

vector<LR1Item> GoTo::operator()(const vector<LR1Item>& I, const GrammarSymbol& X) const {
    vector<LR1Item> goto_I_X;
    for (const auto& existingItem : I) {
        if (existingItem.hasUnvisitedSymbols() && (existingItem.nextUnvisitedSymbol() == X)) {      // [ A -> a.Xb, c ]
            goto_I_X.push_back(existingItem.advance());
        }
    }
    closure(goto_I_X);
    return goto_I_X;
}

}
