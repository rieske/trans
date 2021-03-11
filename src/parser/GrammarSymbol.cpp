#include "GrammarSymbol.h"

namespace parser {

GrammarSymbol::GrammarSymbol(int symbolId, const std::vector<std::size_t>& ruleIndexes) :
        id { symbolId },
        ruleIndexes { ruleIndexes }
{
}

int GrammarSymbol::getId() const {
    return id;
}

bool GrammarSymbol::isTerminal() const {
    return ruleIndexes.empty();
}

bool GrammarSymbol::isNonterminal() const {
    return !isTerminal();
}

const std::vector<std::size_t>& GrammarSymbol::getRuleIndexes() const {
    return ruleIndexes;
}

bool operator<(const GrammarSymbol& lhs, const GrammarSymbol& rhs) {
    return lhs.getId() < rhs.getId();
}

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol) {
    return ostream << symbol.getId();
}

bool operator==(const GrammarSymbol& lhs, const GrammarSymbol& rhs) {
    return lhs.getId() == rhs.getId();
}

} // namespace parser

