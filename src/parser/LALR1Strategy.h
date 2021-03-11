#ifndef LALR1CANONICALCOLLECTION_H_
#define LALR1CANONICALCOLLECTION_H_

#include <map>
#include <vector>

#include "CanonicalCollectionStrategy.h"

namespace parser {

class LALR1Strategy: public CanonicalCollectionStrategy {
public:
    virtual ~LALR1Strategy();

    void computeCanonicalCollection(
            std::vector<std::vector<LR1Item>>& canonicalCollection,
            std::map<std::pair<std::size_t, int>, std::size_t>& computedGotos,
            const std::vector<GrammarSymbol>& grammarSymbols,
            const GoTo& goTo) const override;
};

} // namespace parser

#endif // LALR1CANONICALCOLLECTION_H_
