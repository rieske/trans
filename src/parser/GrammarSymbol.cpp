#include "GrammarSymbol.h"

namespace parser {

GrammarSymbol::GrammarSymbol(int symbolId, const std::vector<int>& ruleIndexes) :
        id { symbolId },
        ruleIndexes { ruleIndexes }
{
}

int GrammarSymbol::getId() const {
    return id;
}

const std::vector<int>& GrammarSymbol::getRuleIndexes() const {
    return ruleIndexes;
}

} // namespace parser

