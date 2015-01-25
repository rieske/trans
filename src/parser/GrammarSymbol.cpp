#include "GrammarSymbol.h"

namespace parser {

GrammarSymbol::GrammarSymbol(const std::string& definition, const std::vector<std::size_t>& ruleIndexes) :
        definition { definition },
        ruleIndexes { ruleIndexes }
{
}

std::string GrammarSymbol::getDefinition() const {
    return definition;
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

}

