#include "GoTo.h"

#include <algorithm>
#include <iterator>

using std::vector;

namespace parser {

GoTo::GoTo(const Closure& closure) :
		closure { closure } {
}

GoTo::~GoTo() {
}

vector<LR1Item> GoTo::operator()(const vector<LR1Item>& I, const GrammarSymbol* X) const {
	vector<LR1Item> goto_I_X;
	for (const auto& existingItem : I) {
		const auto& expectedSymbols = existingItem.getExpectedSymbols();
		if ((!expectedSymbols.empty()) && (expectedSymbols.at(0) == X)) {      // [ A -> a.Xb, c ]
			LR1Item item { existingItem };
			item.advance();

			// XXX: is the if required?
			if (std::find(goto_I_X.begin(), goto_I_X.end(), item) == goto_I_X.end()) {
				goto_I_X.push_back(item);
			}
		}
	}
	closure(goto_I_X);
	return goto_I_X;
}

}
