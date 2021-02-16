#ifndef CANONICALCOLLECTION_H_
#define CANONICALCOLLECTION_H_

#include <map>
#include <string>
#include <vector>

#include "parser/FirstTable.h"
#include "parser/LR1Item.h"
#include "parser/CanonicalCollectionStrategy.h"

namespace parser {

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
    std::map<std::pair<std::size_t, std::string>, std::size_t> computedGotos { };
};

} // namespace parser

#endif // CANONICALCOLLECTION_H_
