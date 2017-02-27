#ifndef LALR1CANONICALCOLLECTION_H_
#define LALR1CANONICALCOLLECTION_H_

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

class LALR1Strategy: public CanonicalCollectionStrategy {
public:
    virtual ~LALR1Strategy();

    void computeCanonicalCollection(
            std::vector<std::vector<LR1Item>>& canonicalCollection,
            std::map<std::pair<std::size_t, std::string>, std::size_t>& computedGotos,
            const std::vector<GrammarSymbol>& grammarSymbols,
            const GoTo& goTo) const override;
};

} /* namespace parser */

#endif /* LALR1CANONICALCOLLECTION_H_ */
