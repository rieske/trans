#ifndef LR1_CANONICALCOLLECTION_H_
#define LR1_CANONICALCOLLECTION_H_

#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "CanonicalCollectionStrategy.h"
#include "GoTo.h"
#include "GrammarSymbol.h"
#include "LR1Item.h"

namespace parser {

class Grammar;

class LR1Strategy: public CanonicalCollectionStrategy {
public:
    virtual ~LR1Strategy();

    void computeCanonicalCollection(
            std::vector<std::vector<LR1Item>>& canonicalCollection,
            std::map<std::pair<std::size_t, std::string>, std::size_t>& computedGotos,
            const std::vector<const GrammarSymbol*>& grammarSymbols,
            const GoTo& goTo) const override;
};

}

#endif /* LR1_CANONICALCOLLECTION_H_ */
