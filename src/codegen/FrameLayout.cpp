#include "FrameLayout.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "FrameSymbol.h"
#include "types/ObjectAbi.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "Instruction.h"
#include "SymbolRefs.h"

namespace codegen {
namespace {

void noteLive(std::map<std::string, std::pair<int, int>>& live, const std::string& name, int i) {
    if (name.empty()) {
        return;
    }
    auto it = live.find(name);
    if (it == live.end()) {
        live.emplace(name, std::make_pair(i, i));
    } else {
        // Only extend; never shrink (PARAM→CALL re-pass after address-of fixpoint).
        if (i < it->second.first) {
            it->second.first = i;
        }
        if (i > it->second.second) {
            it->second.second = i;
        }
    }
}

void noteAllMentions(std::map<std::string, std::pair<int, int>>& live,
        const SymbolRefs& refs, int i) {
    for (const auto& name : refs.uses) {
        noteLive(live, name, i);
    }
    for (const auto& name : refs.defs) {
        noteLive(live, name, i);
    }
}

void applyParamCallExtension(const std::vector<Instruction>& flat,
        std::map<std::string, std::pair<int, int>>& live) {
    // PARAM only queues a value; CALL actually reads it. Extend PARAM operands'
    // live range through the next CALL so finishInstruction does not discard them.
    std::vector<std::string> pendingParams;
    for (int i = 0; i < static_cast<int>(flat.size()); ++i) {
        SymbolRefs refs;
        collectSymbolRefs(flat[static_cast<std::size_t>(i)], refs);
        if (refs.isParam) {
            for (const auto& name : refs.uses) {
                pendingParams.push_back(name);
            }
        }
        if (refs.isCall) {
            for (const auto& name : pendingParams) {
                noteLive(live, name, i);
            }
            pendingParams.clear();
        }
    }
}

// Object --AddressOf/Lea Index/Field--> pointer --Assign--> copy.
// First-wins: each dest binds once. Finite because dest names are finite.
std::map<std::string, std::string> buildAddressAliases(const std::vector<Instruction>& body) {
    std::map<std::string, std::string> aliasToObject;
    bool grew = true;
    while (grew) {
        grew = false;
        for (const auto& q : body) {
            SymbolRefs refs;
            collectSymbolRefs(q, refs);
            if (refs.defs.empty()) {
                continue;
            }
            const std::string& dest = refs.defs.front();
            std::string object;
            if (!refs.addressOfBase.empty()) {
                auto it = aliasToObject.find(refs.addressOfBase);
                object = it == aliasToObject.end() ? refs.addressOfBase : it->second;
            } else if (!refs.assignCopyFrom.empty()) {
                auto it = aliasToObject.find(refs.assignCopyFrom);
                if (it == aliasToObject.end()) {
                    continue;
                }
                object = it->second;
            } else {
                continue;
            }
            if (dest.empty() || dest == object) {
                continue;
            }
            if (aliasToObject.emplace(dest, object).second) {
                grew = true;
            }
        }
    }
    return aliasToObject;
}

bool extendLiveThroughAliases(
        std::map<std::string, std::pair<int, int>>& live,
        const std::map<std::string, std::string>& aliasToObject) {
    bool changed = false;
    for (const auto& [ptr, object] : aliasToObject) {
        auto resIt = live.find(ptr);
        if (resIt == live.end()) {
            continue;
        }
        auto opIt = live.find(object);
        if (opIt == live.end()) {
            live.emplace(object, resIt->second);
            changed = true;
            continue;
        }
        if (opIt->second.second < resIt->second.second) {
            opIt->second.second = resIt->second.second;
            changed = true;
        }
        if (resIt->second.first < opIt->second.first) {
            opIt->second.first = resIt->second.first;
            changed = true;
        }
    }
    return changed;
}

} // namespace

std::vector<Value> packFrameValues(
        const std::map<std::string, symbols::ValueEntry>& locals,
        const std::vector<Instruction>& body) {

    // Keep FrameSymbol through packing; project to Value only at emission via toValueAtSlot.
    std::vector<FrameSymbol> symbols;
    symbols.reserve(locals.size());
    for (const auto& entry : locals) {
        if (entry.second.isGlobal()) {
            continue;
        }
        symbols.push_back(frameSymbolFrom(entry.second, entry.second.isExpressionTemp()));
    }

    const std::vector<Instruction>& flat = body;
    // first/last mention index among body instructions; temps with no mention still
    // get a slot (defensive - should not happen for real temps).
    std::map<std::string, std::pair<int, int>> live;
    for (int i = 0; i < static_cast<int>(flat.size()); ++i) {
        SymbolRefs refs;
        collectSymbolRefs(flat[static_cast<std::size_t>(i)], refs);
        noteAllMentions(live, refs, i);
    }
    applyParamCallExtension(flat, live);

    const auto aliasToObject = buildAddressAliases(flat);
    for (int pass = 0; pass < 4; ++pass) {
        if (!extendLiveThroughAliases(live, aliasToObject)) {
            break;
        }
        applyParamCallExtension(flat, live);
    }

    std::vector<Value> values;
    int nextSlot = 0;
    // Named (non-temp) locals: permanent non-overlapping slots for the whole frame.
    for (const auto& fs : symbols) {
        if (fs.isTemp) {
            continue;
        }
        const int words = type::object_abi::valueWords(fs.sizeBytes());
        values.push_back(fs.toValueAtSlot(nextSlot));
        nextSlot += words;
    }

    struct Interval {
        FrameSymbol fs;
        int words;
        int first;
        int last;
        int slot { -1 };
    };
    std::vector<Interval> intervals;
    for (const auto& fs : symbols) {
        if (!fs.isTemp) {
            continue;
        }
        Interval iv;
        iv.fs = fs;
        iv.words = type::object_abi::valueWords(fs.sizeBytes());
        auto it = live.find(fs.name);
        if (it == live.end()) {
            iv.first = 0;
            iv.last = 0;
        } else {
            iv.first = it->second.first;
            iv.last = it->second.second;
        }
        intervals.push_back(std::move(iv));
    }
    std::sort(intervals.begin(), intervals.end(),
            [](const Interval& a, const Interval& b) {
                if (a.first != b.first) {
                    return a.first < b.first;
                }
                return a.fs.name < b.fs.name;
            });

    // Free list of spill-slot word indices in the temp region. Multi-word temps
    // free every word on expiry so later multi-word (or single) temps can reuse
    // contiguous runs - without this, tree walk frames stay multi-KB and deep
    // recursion SEGVs before core.maxtreedepth (git t6700 archive/diff-tree).
    std::vector<int> freeSlots;
    struct Active {
        int last;
        int slot;
        int words;
    };
    std::vector<Active> active;

    auto takeContiguous = [&freeSlots](int words) -> int {
        if (words <= 0 || freeSlots.empty()) {
            return -1;
        }
        std::sort(freeSlots.begin(), freeSlots.end());
        freeSlots.erase(std::unique(freeSlots.begin(), freeSlots.end()), freeSlots.end());
        for (std::size_t i = 0; i + static_cast<std::size_t>(words) <= freeSlots.size(); ++i) {
            const int start = freeSlots[i];
            bool ok = true;
            for (int w = 1; w < words; ++w) {
                if (freeSlots[i + static_cast<std::size_t>(w)] != start + w) {
                    ok = false;
                    break;
                }
            }
            if (!ok) {
                continue;
            }
            freeSlots.erase(freeSlots.begin() + static_cast<std::ptrdiff_t>(i),
                    freeSlots.begin() + static_cast<std::ptrdiff_t>(i + words));
            return start;
        }
        return -1;
    };

    for (auto& iv : intervals) {
        // Expire temps whose last use is before this first use.
        std::vector<Active> stillActive;
        for (const auto& a : active) {
            if (a.last < iv.first) {
                for (int w = 0; w < a.words; ++w) {
                    freeSlots.push_back(a.slot + w);
                }
            } else {
                stillActive.push_back(a);
            }
        }
        active.swap(stillActive);

        int reused = takeContiguous(iv.words);
        if (reused >= 0) {
            iv.slot = reused;
        } else {
            iv.slot = nextSlot;
            nextSlot += iv.words;
        }
        active.push_back(Active { iv.last, iv.slot, iv.words });
        Value tempValue = iv.fs.toValueAtSlot(iv.slot);
        tempValue.setLastUseOrdinal(iv.last);
        values.push_back(std::move(tempValue));
    }

    return values;

}

} // namespace codegen
