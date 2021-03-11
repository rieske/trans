#include "GrammarSymbol.h"

namespace parser {

GrammarSymbol::GrammarSymbol(int symbolId, const std::vector<std::size_t>& ruleIndexes) :
        id { symbolId },
        ruleIndexes { ruleIndexes }
{
}

std::string GrammarSymbol::getDefinition() const {
    // Temporary conversion. Migrate after testing grammar construction with integer definitions
    return std::to_string(id);
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
    return lhs.getDefinition() < rhs.getDefinition();
}

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol) {
    return ostream << symbol.getDefinition();
}

bool operator ==(const GrammarSymbol& lhs, const GrammarSymbol& rhs) {
    return lhs.getDefinition() == rhs.getDefinition();
}

} // namespace parser

