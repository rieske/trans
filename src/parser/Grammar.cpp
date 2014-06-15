#include "Grammar.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>

#include "FirstTable.h"
#include "Closure.h"

#include "BNFReader.h"

using std::shared_ptr;
using std::unique_ptr;
using std::vector;

Grammar::Grammar(const vector<shared_ptr<const GrammarSymbol>> terminals, const vector<shared_ptr<const GrammarSymbol>> nonterminals) :
		terminals { terminals },
		nonterminals { nonterminals },
		startSymbol { std::make_shared<GrammarSymbol>("<__start__>", 0) },
		endSymbol { std::make_shared<GrammarSymbol>("'$end$'", 0) } {
	startSymbol->addProduction( { nonterminals.at(0) });
	this->terminals.push_back(endSymbol);
}

Grammar::~Grammar() {
}

LR1Item Grammar::getReductionById(size_t nonterminalId, size_t productionId) const {
	const auto& nonterminalIterator = std::find_if(nonterminals.begin(), nonterminals.end(),
			[&nonterminalId] (shared_ptr<const GrammarSymbol> nonterminal) {return nonterminal->getId() == nonterminalId;});
	if (nonterminalIterator == nonterminals.end()) {
		throw std::invalid_argument("Nonterminal not found by id " + nonterminalId);
	}
	return {*nonterminalIterator, productionId, {}};
}

std::ostream& operator<<(std::ostream& out, const Grammar& grammar) {
	out << "\nTerminals:\n";
	for (auto& terminal : grammar.terminals) {
		out << *terminal << "\n";
	}
	out << "\nNonterminals:\n";
	for (auto& nonterminal : grammar.nonterminals) {
		out << *nonterminal << "\n";
	}
	return out;
}
