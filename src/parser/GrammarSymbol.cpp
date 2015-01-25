#include "GrammarSymbol.h"

using std::string;
using std::vector;

namespace parser {

GrammarSymbol::GrammarSymbol(const string& definition) :
        definition { definition }
{
}

GrammarSymbol::~GrammarSymbol() {
}

string GrammarSymbol::getDefinition() const {
    return definition;
}

bool GrammarSymbol::isTerminal() const {
    return ruleIndexes.empty();
}

bool GrammarSymbol::isNonterminal() const {
    return !isTerminal();
}

void GrammarSymbol::addRuleIndex(std::size_t ruleIndex) {
    ruleIndexes.push_back(ruleIndex);
}

const std::vector<std::size_t>& GrammarSymbol::getRuleIndexes() const {
    return ruleIndexes;
}

bool operator <(const GrammarSymbol& lhs, const GrammarSymbol& rhs) {
    return lhs.getDefinition() < rhs.getDefinition();
}

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol) {
    return ostream << symbol.getDefinition();
}

bool operator ==(const GrammarSymbol& lhs, const GrammarSymbol& rhs) {
    return lhs.getDefinition() == rhs.getDefinition();
}

}

