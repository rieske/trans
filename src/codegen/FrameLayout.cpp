#include "FrameLayout.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <vector>

#include "types/ObjectAbi.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "quadruples/Quadruple.h"

namespace codegen {
namespace {

bool isExpressionTempName(const std::string& name) {
    if (name.size() < 3 || name[0] != '$' || name[1] != 't') {
        return false;
    }
    for (std::size_t i = 2; i < name.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(name[i]))) {
            return false;
        }
    }
    return true;
}

void flattenQuadruples(const std::vector<std::unique_ptr<Quadruple>>& quads,
        std::vector<const Quadruple*>& out) {
    for (const auto& q : quads) {
        if (const auto* block = dynamic_cast<const BasicBlock*>(q.get())) {
            flattenQuadruples(block->getInstructions(), out);
        } else {
            out.push_back(q.get());
        }
    }
}

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

void applyParamCallExtension(const std::vector<const Quadruple*>& flat,
        std::map<std::string, std::pair<int, int>>& live) {
    // PARAM only queues a value; CALL actually reads it. Extend PARAM operands'
    // live range through the next CALL so finishInstruction does not discard them.
    std::vector<std::string> pendingParams;
    for (int i = 0; i < static_cast<int>(flat.size()); ++i) {
        SymbolRefs refs;
        flat[static_cast<std::size_t>(i)]->collectSymbolRefs(refs);
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

} // namespace

std::vector<Value> packFrameValues(
        const std::map<std::string, semantic_analyzer::ValueEntry>& locals,
        const std::vector<std::unique_ptr<BasicBlock>>& body) {

    struct LocalInfo {
        std::string name;
        int sizeBytes;
        int words;
        Type type;
        bool isSigned;
        bool isTemp;
    };
    std::vector<LocalInfo> infos;
    infos.reserve(locals.size());
    for (const auto& entry : locals) {
        if (entry.second.isGlobal()) {
            continue;
        }
        const type::Type& ty = entry.second.getType();
        LocalInfo info;
        info.name = entry.second.getName();
        info.sizeBytes = type::valueSizeBytes(ty);
        info.words = object_abi::valueWords(info.sizeBytes);
        info.type = type::isFloating(ty) ? Type::FLOATING : Type::INTEGRAL;
        info.isSigned = type::valueIsSigned(ty);
        info.isTemp = isExpressionTempName(info.name);
        infos.push_back(std::move(info));
    }

    std::vector<const Quadruple*> flat;
    for (const auto& bb : body) {
        flattenQuadruples(bb->getInstructions(), flat);
    }
    // first/last mention index among body quadruples; temps with no mention still
    // get a slot (defensive - should not happen for real temps).
    std::map<std::string, std::pair<int, int>> live;
    for (int i = 0; i < static_cast<int>(flat.size()); ++i) {
        SymbolRefs refs;
        flat[static_cast<std::size_t>(i)]->collectSymbolRefs(refs);
        noteAllMentions(live, refs, i);
    }
    applyParamCallExtension(flat, live);

    // Address-of aliases the object: AddressOf(base) → result (explicit def).
    // Extend the object's live range through the pointer's last use so later
    // temps cannot reclaim multi-word compound-literal / array storage while a
    // derived pointer is still live (e.g. sizeof temps between &arr and CALL).
    auto extendAddressOfBases = [&]() {
        bool changed = false;
        for (const auto* q : flat) {
            SymbolRefs refs;
            q->collectSymbolRefs(refs);
            if (refs.addressOfBase.empty() || refs.defs.empty()) {
                continue;
            }
            const std::string& operand = refs.addressOfBase;
            const std::string& result = refs.defs.front();
            if (result.empty() || result == operand) {
                continue;
            }
            auto resIt = live.find(result);
            if (resIt == live.end()) {
                continue;
            }
            auto opIt = live.find(operand);
            if (opIt == live.end()) {
                live.emplace(operand, resIt->second);
                changed = true;
            } else if (opIt->second.second < resIt->second.second) {
                opIt->second.second = resIt->second.second;
                changed = true;
            }
            auto opIt2 = live.find(operand);
            if (opIt2 != live.end() && resIt->second.first < opIt2->second.first) {
                opIt2->second.first = resIt->second.first;
                changed = true;
            }
        }
        return changed;
    };
    // Fixpoint: pointer live ranges may grow via PARAM→CALL after base extension.
    for (int pass = 0; pass < 4; ++pass) {
        if (!extendAddressOfBases()) {
            break;
        }
        applyParamCallExtension(flat, live);
    }

    std::vector<Value> values;
    int nextSlot = 0;
    // Named (non-temp) locals: permanent non-overlapping slots for the whole frame.
    for (const auto& info : infos) {
        if (info.isTemp) {
            continue;
        }
        values.push_back(Value { info.name, nextSlot, info.type, info.sizeBytes, info.isSigned });
        nextSlot += info.words;
    }

    struct Interval {
        std::string name;
        int sizeBytes;
        int words;
        Type type;
        bool isSigned;
        int first;
        int last;
        int slot { -1 };
    };
    std::vector<Interval> intervals;
    for (const auto& info : infos) {
        if (!info.isTemp) {
            continue;
        }
        Interval iv;
        iv.name = info.name;
        iv.sizeBytes = info.sizeBytes;
        iv.words = info.words;
        iv.type = info.type;
        iv.isSigned = info.isSigned;
        auto it = live.find(info.name);
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
                return a.name < b.name;
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
    const int tempBase = nextSlot;

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
        Value tempValue { iv.name, iv.slot, iv.type, iv.sizeBytes, iv.isSigned };
        tempValue.setLastUseOrdinal(iv.last);
        values.push_back(std::move(tempValue));
    }
    (void)tempBase;

    return values;

}

} // namespace codegen
