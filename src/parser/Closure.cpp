#include "Closure.h"
#include "FirstTable.h"

#include <algorithm>

using std::vector;
using std::shared_ptr;

Closure::Closure(const FirstTable& first) :
		first { first } {
}

Closure::~Closure() {
}

void Closure::operator()(vector<LR1Item>& items) const {
	bool more = true;
	while (more) {
		more = false;
		for (size_t i = 0; i < items.size(); ++i) {
			const LR1Item& item = items.at(i);
			const auto& expectedSymbols = item.getExpected();
			if (!expectedSymbols.empty() && expectedSymbols.at(0)->isNonterminal()) { // [ A -> u.Bv, a ] (expected[0] == B)
				const auto& nextExpectedNonterminal = expectedSymbols.at(0);
				vector<shared_ptr<const GrammarSymbol>> firstForNextSymbol {
						(expectedSymbols.size() > 1) ? first(expectedSymbols.at(1)) : item.getLookaheads() };
				for (size_t productionId = 0; productionId < nextExpectedNonterminal->getProductions().size(); ++productionId) {
					LR1Item newItem { nextExpectedNonterminal, productionId, firstForNextSymbol };
					const auto& existingItemIt = std::find_if(items.begin(), items.end(),
							[&newItem] (const LR1Item& existingItem) {return existingItem.coresAreEqual(newItem);});
					if (existingItemIt == items.end()) {
						items.push_back(newItem);
						more = true;
					} else {
						existingItemIt->mergeLookaheads(newItem.getLookaheads());
					}
				}
			}
		}
	}
}
