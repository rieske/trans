#include "Grammar.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>

#include "FirstTable.h"
#include "Closure.h"

using std::unique_ptr;
using std::vector;
using std::string;

Grammar::~Grammar() {
}

const GrammarSymbol* Grammar::getStartSymbol() const {
	return startSymbol.get();
}

const GrammarSymbol* Grammar::getEndSymbol() const {
	return endSymbol.get();
}

LR1Item Grammar::getReductionById(string nonterminalId, size_t productionId) const {
	const auto& nonterminals = getNonterminals();
	const auto& nonterminalIterator = std::find_if(nonterminals.begin(), nonterminals.end(),
			[&nonterminalId] (const GrammarSymbol* nonterminal) {return nonterminal->getName() == nonterminalId;});
	if (nonterminalIterator == nonterminals.end()) {
		throw std::invalid_argument("Nonterminal not found by id " + nonterminalId);
	}
	return {*nonterminalIterator, productionId, {}};
}

std::ostream& operator<<(std::ostream& out, const Grammar& grammar) {
	out << "\nTerminals:\n";
	for (auto& terminal : grammar.getTerminals()) {
		out << *terminal << "\n";
	}
	out << "\nNonterminals:\n";
	for (auto& nonterminal : grammar.getNonterminals()) {
		out << *nonterminal << "\n";
	}
	return out;
}
