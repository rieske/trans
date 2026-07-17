#include "CanonicalCollection.h"

#include "util/LogManager.h"
#include "util/Logger.h"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

Logger& logger = LogManager::getComponentLogger(Component::PARSER);

using namespace parser;


void closeItemSet(
        const FirstTable& first,
        const Grammar& grammar,
        GenerationBuffers& buffers,
        std::vector<LR1Item>& items) {
    auto& coreIndex = buffers.coreIndex;
    auto& worklist = buffers.worklist;
    buffers.prepareClosure(items.size());

    for (std::size_t i = 0; i < items.size(); ++i) {
        coreIndex.emplace(items[i].coreKey(), i);
        worklist.push_back(i);
    }
    items.reserve(std::max(items.capacity(), items.size() + 64));

    std::size_t head = 0;
    while (head < worklist.size()) {
        const std::size_t index = worklist[head++];
        if (!items[index].hasUnvisitedSymbols() || grammar.isTerminal(items[index].nextUnvisitedSymbol())) {
            continue;
        }

        const int B = items[index].nextUnvisitedSymbol();
        const LR1Item::LookaheadSet propagated = items[index].hasSymbolsAfterNext()
                ? first.firstBits(items[index].symbolAfterNext())
                : items[index].lookaheads();

        for (const auto& production : grammar.getProductionsOfSymbol(B)) {
            const std::uint64_t key =
                    (static_cast<std::uint64_t>(static_cast<std::uint32_t>(production.getId())) << 32);
            const auto existing = coreIndex.find(key);
            if (existing == coreIndex.end()) {
                items.emplace_back(production, propagated);
                const std::size_t newIndex = items.size() - 1;
                coreIndex.emplace(key, newIndex);
                worklist.push_back(newIndex);
            } else if (items[existing->second].mergeLookaheads(propagated)) {
                worklist.push_back(existing->second);
            }
        }
    }

    // Closed sets are sorted by core so LALR merge is a linear zip and
    // core signatures need no extra sort.
    std::sort(items.begin(), items.end(), [](const LR1Item& left, const LR1Item& right) {
        return left.coreKey() < right.coreKey();
    });
}

std::unordered_map<int, std::vector<LR1Item>> gotoAllFrom(
        const std::vector<LR1Item>& I,
        const FirstTable& first,
        const Grammar& grammar,
        GenerationBuffers& buffers) {
    std::unordered_map<int, std::vector<LR1Item>> bySymbol;
    bySymbol.reserve(I.size());
    for (const auto& item : I) {
        if (item.hasUnvisitedSymbols()) {
            bySymbol[item.nextUnvisitedSymbol()].push_back(item.advance());
        }
    }
    for (auto& entry : bySymbol) {
        entry.second.reserve(entry.second.size() + 16);
        closeItemSet(first, grammar, buffers, entry.second);
    }
    return bySymbol;
}

// --- LALR(1) ---

// items must be sorted by coreKey (closeItemSet invariant).
std::vector<std::uint64_t> makeCoreSignature(const std::vector<LR1Item>& items, GenerationBuffers& buffers) {
    auto& signature = buffers.coreSignature;
    signature.clear();
    signature.reserve(items.size());
    for (const auto& item : items) {
        signature.push_back(item.coreKey());
    }
    return signature;
}

struct CoreSignatureHash {
    std::size_t operator()(const std::vector<std::uint64_t>& signature) const noexcept {
        std::size_t hash = signature.size();
        for (const std::uint64_t key : signature) {
            hashCombine(hash, key);
        }
        return hash;
    }
};

// Both sides sorted by the same cores (LALR state with matching core signature).
bool mergeLookaheadsByCore(
        std::vector<LR1Item>& existing,
        const std::vector<LR1Item>& incoming) {
    bool merged = false;
    for (std::size_t i = 0; i < existing.size(); ++i) {
        merged |= existing[i].mergeLookaheads(incoming[i].lookaheads());
    }
    return merged;
}

// Deterministic processing order: only symbols with a non-empty goto, sorted by id.
std::vector<int> sortedGotoSymbols(const std::unordered_map<int, std::vector<LR1Item>>& gotos) {
    std::vector<int> symbols;
    symbols.reserve(gotos.size());
    for (const auto& entry : gotos) {
        if (!entry.second.empty()) {
            symbols.push_back(entry.first);
        }
    }
    std::sort(symbols.begin(), symbols.end());
    return symbols;
}

void computeLALR1(
        std::vector<std::vector<LR1Item>>& canonicalCollection,
        GotoMap& computedGotos,
        const FirstTable& first,
        const Grammar& grammar,
        GenerationBuffers& buffers) {
    std::unordered_map<std::vector<std::uint64_t>, std::size_t, CoreSignatureHash> stateByCores;
    stateByCores.reserve(512);
    stateByCores.emplace(makeCoreSignature(canonicalCollection[0], buffers), 0);
    computedGotos.reserve(512);

    std::deque<std::size_t> worklist { 0 };
    std::vector<char> inQueue(1, 1);

    auto enqueue = [&](std::size_t state) {
        if (state >= inQueue.size()) {
            inQueue.resize(state + 1, 0);
        }
        if (!inQueue[state]) {
            inQueue[state] = 1;
            worklist.push_back(state);
        }
    };

    while (!worklist.empty()) {
        const std::size_t state = worklist.front();
        worklist.pop_front();
        inQueue[state] = 0;

        auto gotos = gotoAllFrom(canonicalCollection[state], first, grammar, buffers);
        for (const int X : sortedGotoSymbols(gotos)) {
            auto& gotoSet = gotos[X];
            auto signature = makeCoreSignature(gotoSet, buffers);
            const auto existing = stateByCores.find(signature);
            if (existing == stateByCores.end()) {
                const std::size_t newState = canonicalCollection.size();
                canonicalCollection.push_back(std::move(gotoSet));
                stateByCores.emplace(std::move(signature), newState);
                computedGotos[{ state, X }] = newState;
                enqueue(newState);
            } else {
                const std::size_t target = existing->second;
                computedGotos[{ state, X }] = target;
                if (mergeLookaheadsByCore(canonicalCollection[target], gotoSet)) {
                    enqueue(target);
                }
            }
        }
    }
}

// --- LR(1) ---

using ItemSetSignature = std::vector<std::pair<std::uint64_t, LR1Item::LookaheadSet>>;

// items must be sorted by coreKey (closeItemSet invariant).
ItemSetSignature makeItemSetSignature(const std::vector<LR1Item>& items) {
    ItemSetSignature signature;
    signature.reserve(items.size());
    for (const auto& item : items) {
        signature.emplace_back(item.coreKey(), item.lookaheads());
    }
    return signature;
}

struct ItemSetSignatureHash {
    std::size_t operator()(const ItemSetSignature& signature) const noexcept {
        std::size_t hash = signature.size();
        for (const auto& entry : signature) {
            hashCombine(hash, entry.first);
            for (std::size_t i = 0; i < entry.second.size(); i += 64) {
                unsigned long long word = 0;
                for (std::size_t b = 0; b < 64 && i + b < entry.second.size(); ++b) {
                    if (entry.second.test(i + b)) {
                        word |= 1ULL << b;
                    }
                }
                hashCombine(hash, word);
            }
        }
        return hash;
    }
};

void computeLR1(
        std::vector<std::vector<LR1Item>>& canonicalCollection,
        GotoMap& computedGotos,
        const FirstTable& first,
        const Grammar& grammar,
        GenerationBuffers& buffers) {
    std::unordered_map<ItemSetSignature, std::size_t, ItemSetSignatureHash> stateByItems;
    stateByItems.reserve(256);
    stateByItems.emplace(makeItemSetSignature(canonicalCollection[0]), 0);

    for (std::size_t i = 0; i < canonicalCollection.size(); ++i) {
        auto gotos = gotoAllFrom(canonicalCollection[i], first, grammar, buffers);
        for (const int X : sortedGotoSymbols(gotos)) {
            auto& gotoSet = gotos[X];
            auto signature = makeItemSetSignature(gotoSet);
            const auto existing = stateByItems.find(signature);
            if (existing == stateByItems.end()) {
                const std::size_t newState = canonicalCollection.size();
                canonicalCollection.push_back(std::move(gotoSet));
                stateByItems.emplace(std::move(signature), newState);
                computedGotos[{ i, X }] = newState;
            } else {
                computedGotos[{ i, X }] = existing->second;
            }
        }
    }
}

} // namespace

namespace parser {

CanonicalCollection::CanonicalCollection(
        const FirstTable& first,
        const Grammar& grammar,
        AutomatonKind kind)
{
    std::vector<LR1Item> initialSet {
        LR1Item { grammar.getTopRule(), std::vector<int>{ grammar.getEndSymbol() }, grammar }
    };
    closeItemSet(first, grammar, buffers, initialSet);
    canonicalCollection.push_back(std::move(initialSet));

    if (kind == AutomatonKind::LALR1) {
        computeLALR1(canonicalCollection, computedGotos, first, grammar, buffers);
    } else {
        computeLR1(canonicalCollection, computedGotos, first, grammar, buffers);
    }

    logCollection(grammar);
}

std::size_t CanonicalCollection::stateCount() const noexcept {
    return canonicalCollection.size();
}

const std::vector<LR1Item>& CanonicalCollection::setOfItemsAtState(size_t state) const {
    return canonicalCollection.at(state);
}

std::size_t CanonicalCollection::goTo(std::size_t stateFrom, int symbol) const {
    return computedGotos.at({ stateFrom, symbol });
}

void CanonicalCollection::logCollection(const Grammar& grammar) const {
    if (logger.isNull()) {
        return;
    }
    logger << "\n*********************\nCanonical collection:\n*********************\n";
    int setNo = 0;
    for (const auto& setOfItems : canonicalCollection) {
        logger << "Set " << setNo++ << ":\n";
        for (const auto& item : setOfItems) {
            logger << item.str(grammar);
        }
        logger << "\n";
    }
    logger << "Computed GOTOs:\n";
    for (const auto& g : computedGotos) {
        logger << g.first.first << "\t" << g.first.second << "\t" << g.second << "\n";
    }
}

} // namespace parser
