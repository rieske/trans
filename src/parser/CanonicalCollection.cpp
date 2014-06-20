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

CanonicalCollection::CanonicalCollection(const FirstTable& firstTable) :
		firstTable { firstTable } {
}

CanonicalCollection::~CanonicalCollection() {
}

vector<vector<LR1Item>> CanonicalCollection::computeForGrammar(const Grammar& grammar) const {

	vector<const GrammarSymbol*> grammarSymbols;
	for (const auto& terminal : grammar.getTerminals()) {
		grammarSymbols.push_back(terminal);
	}
	for (const auto& nonterminal : grammar.getNonterminals()) {
		grammarSymbols.push_back(nonterminal);
	}

	LR1Item initialItem { grammar.getStartSymbol(), 0, { grammar.getEndSymbol() } };
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
