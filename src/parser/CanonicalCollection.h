#ifndef CANONICALCOLLECTION_H_
#define CANONICALCOLLECTION_H_

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "parser/FirstTable.h"
#include "parser/GenerationBuffers.h"
#include "parser/HashCombine.h"
#include "parser/LR1Item.h"

namespace parser {

enum class AutomatonKind { LALR1, LR1 };

using GotoMap = std::unordered_map<StateSymbolKey, std::size_t, StateSymbolHash>;

class CanonicalCollection {
public:
    CanonicalCollection(const FirstTable& firstTable, const Grammar& grammar, AutomatonKind kind);

    std::size_t stateCount() const noexcept;
    const std::vector<LR1Item>& setOfItemsAtState(size_t state) const;
    std::size_t goTo(std::size_t stateFrom, int symbol) const;
    // All computed transitions (state, symbol) -> state; used to fill the goto table.
    const GotoMap& computedTransitions() const noexcept { return computedGotos; }

private:
    void logCollection(const Grammar& grammar) const;

    GenerationBuffers buffers;
    std::vector<std::vector<LR1Item>> canonicalCollection;
    GotoMap computedGotos;
};

} // namespace parser

#endif
