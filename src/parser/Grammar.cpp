#include "Grammar.h"

using std::unique_ptr;
using std::vector;
using std::string;

namespace parser {

Grammar::~Grammar() {
}

const GrammarSymbol* Grammar::getStartSymbol() const {
	return startSymbol.get();
}

const GrammarSymbol* Grammar::getEndSymbol() const {
	return endSymbol.get();
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

}
