#include "CanonicalCollection.h"

#include <stddef.h>
#include <algorithm>
#include <iterator>
#include <memory>

#include "Closure.h"
#include "GoTo.h"
#include "Grammar.h"
#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

CanonicalCollection::CanonicalCollection(const FirstTable& firstTable) :
		firstTable { firstTable } {
}

CanonicalCollection::~CanonicalCollection() {
}

vector<vector<LR1Item>> CanonicalCollection::computeForGrammar(const Grammar& grammar) const {

	vector<shared_ptr<const GrammarSymbol>> grammarSymbols;
	grammarSymbols.insert(grammarSymbols.begin(), grammar.terminals.begin(), grammar.terminals.end());
	grammarSymbols.insert(grammarSymbols.begin(), grammar.nonterminals.begin(), grammar.nonterminals.end());

	LR1Item initialItem { grammar.startSymbol, 0, { grammar.endSymbol } };
	vector<LR1Item> initialSet { initialItem };
	Closure closure { firstTable };
	closure(initialSet);
	vector<vector<LR1Item>> canonicalCollection { initialSet };
	GoTo goTo { closure };

	for (size_t i = 0; i < canonicalCollection.size(); ++i) { // for each set of items I in C
		for (const auto& X : grammarSymbols) { // and each grammar symbol X
			const auto& goto_I_X = goTo(canonicalCollection.at(i), X);
			if (!goto_I_X.empty()) { // such that goto(I, X) is not empty
				if (std::find(canonicalCollection.begin(), canonicalCollection.end(), goto_I_X) == canonicalCollection.end()) { // and not in C
					canonicalCollection.push_back(goto_I_X);
				}
			}
		}
	}
	return canonicalCollection;
}
