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

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol) {
    return ostream << symbol.getDefinition();
}

void GrammarSymbol::addRuleIndex(int ruleIndex) {
    ruleIndexes.push_back(ruleIndex);
}

const std::vector<int>& GrammarSymbol::getRuleIndexes() const {
    return ruleIndexes;
}

}

