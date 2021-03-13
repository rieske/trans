#ifndef LR1_CANONICALCOLLECTION_H_
#define LR1_CANONICALCOLLECTION_H_

#include <map>
#include <vector>

#include "CanonicalCollectionStrategy.h"

namespace parser {

class LR1Strategy: public CanonicalCollectionStrategy {
public:
    virtual ~LR1Strategy();

    void computeCanonicalCollection(
            std::vector<std::vector<LR1Item>>& canonicalCollection,
            std::map<std::pair<std::size_t, int>, std::size_t>& computedGotos,
            const std::vector<int>& grammarSymbols,
            const GoTo& goTo) const override;
};

} // namespace parser

#endif // LR1_CANONICALCOLLECTION_H_
