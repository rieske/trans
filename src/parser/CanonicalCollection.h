#ifndef CANONICALCOLLECTION_H_
#define CANONICALCOLLECTION_H_

#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "FirstTable.h"
#include "LR1Item.h"

namespace parser {

class CanonicalCollectionStrategy;

class CanonicalCollection {
public:
    CanonicalCollection(const FirstTable& firstTable, const Grammar& grammar, const CanonicalCollectionStrategy& strategy);
    virtual ~CanonicalCollection();

    std::size_t stateCount() const noexcept;
    const std::vector<LR1Item>& setOfItemsAtState(size_t state) const;
    std::size_t goTo(std::size_t stateFrom, std::string symbol) const;

private:
    void logCollection() const;

    const FirstTable firstTable;

    std::vector<std::vector<LR1Item>> canonicalCollection;
    std::map<std::pair<std::size_t, std::string>, std::size_t> computedGotos;
};

}

#endif /* CANONICALCOLLECTION_H_ */
