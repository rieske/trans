#include "Grammar.h"

namespace parser {

Grammar::~Grammar() {
}

const GrammarSymbol& Grammar::getStartSymbol() const {
    return startSymbol;
}

const GrammarSymbol& Grammar::getEndSymbol() const {
    return endSymbol;
}

std::ostream& operator<<(std::ostream& out, const Grammar& grammar) {
    out << "\nTerminals:\n";
    for (auto& terminal : grammar.getTerminals()) {
        out << terminal << "\n";
    }
    out << "\nNonterminals:\n";
    for (auto& nonterminal : grammar.getNonterminals()) {
        out << nonterminal << "\n";
    }
    return out;
}

} // namespace parser

