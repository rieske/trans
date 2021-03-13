#ifndef CANONICALCOLLECTIONSTRATEGY_H_
#define CANONICALCOLLECTIONSTRATEGY_H_

#include <vector>

#include "GoTo.h"
#include "GrammarSymbol.h"
#include "LR1Item.h"

namespace parser {

class CanonicalCollectionStrategy {
public:
    virtual ~CanonicalCollectionStrategy() {
    }

    virtual void computeCanonicalCollection(
            std::vector<std::vector<LR1Item>>& canonicalCollection,
            std::map<std::pair<std::size_t, int>, std::size_t>& computedGotos,
            const std::vector<int>& grammarSymbols,
            const GoTo& goTo) const = 0;
};

} // namespace parser

#endif // CANONICALCOLLECTIONSTRATEGY_H_
