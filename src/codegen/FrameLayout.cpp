#include "FrameLayout.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include "SymbolRefs.h"
#include "types/ObjectAbi.h"

namespace codegen {
namespace {

void noteLive(std::map<int, std::pair<int, int>>& live, int id, int i) {
    if (id == kNoSymbol) {
        return;
    }
    auto it = live.find(id);
    if (it == live.end()) {
        live.emplace(id, std::make_pair(i, i));
    } else {
        if (i < it->second.first) {
            it->second.first = i;
        }
        if (i > it->second.second) {
            it->second.second = i;
        }
    }
}

std::pair<int, int> liveRange(const std::map<int, std::pair<int, int>>& live, int id) {
    auto it = live.find(id);
    if (it == live.end()) {
        return { 0, 0 };
    }
    return it->second;
}

} // namespace

std::vector<Value> packFrameValues(
        std::vector<Value> locals,
        const std::vector<Instruction>& body) {
    std::map<int, std::pair<int, int>> live;
    std::set<int> addressTaken;
    std::vector<int> pendingParams;
    for (int i = 0; i < static_cast<int>(body.size()); ++i) {
        SymbolRefs refs;
        collectSymbolRefs(body[static_cast<std::size_t>(i)], refs);
        for (int id : refs.uses) {
            noteLive(live, id, i);
        }
        for (int id : refs.defs) {
            noteLive(live, id, i);
        }
        if (refs.addressOfBase != kNoSymbol) {
            addressTaken.insert(refs.addressOfBase);
        }
        if (refs.isParam) {
            pendingParams.insert(pendingParams.end(), refs.uses.begin(), refs.uses.end());
        }
        if (refs.isCall) {
            for (int id : pendingParams) {
                noteLive(live, id, i);
            }
            pendingParams.clear();
        }
    }

    auto pinned = [&](const Value& local) {
        if (!local.isExpressionTemp()) {
            return true;
        }
        if (type::object_abi::valueWords(local.getSizeInBytes()) > 1) {
            return true;
        }
        return addressTaken.count(local.id()) != 0;
    };

    std::vector<Value> values;
    int nextSlot = 0;
    for (const auto& local : locals) {
        if (!pinned(local)) {
            continue;
        }
        const int words = type::object_abi::valueWords(local.getSizeInBytes());
        Value home = local.withIndex(nextSlot);
        if (local.isExpressionTemp()) {
            home.setLastUseOrdinal(liveRange(live, local.id()).second);
        }
        values.push_back(std::move(home));
        nextSlot += words;
    }

    struct Interval {
        Value value;
        int first;
        int last;
    };
    std::vector<Interval> intervals;
    for (auto& local : locals) {
        if (pinned(local)) {
            continue;
        }
        const auto range = liveRange(live, local.id());
        intervals.push_back(Interval { std::move(local), range.first, range.second });
    }
    std::sort(intervals.begin(), intervals.end(),
            [](const Interval& a, const Interval& b) {
                if (a.first != b.first) {
                    return a.first < b.first;
                }
                return a.value.id() < b.value.id();
            });

    std::vector<int> freeSlots;
    struct Active {
        int last;
        int slot;
    };
    std::vector<Active> active;

    for (auto& iv : intervals) {
        std::vector<Active> still;
        for (const auto& a : active) {
            if (a.last < iv.first) {
                freeSlots.push_back(a.slot);
            } else {
                still.push_back(a);
            }
        }
        active.swap(still);

        int slot = nextSlot;
        if (!freeSlots.empty()) {
            slot = freeSlots.back();
            freeSlots.pop_back();
        } else {
            ++nextSlot;
        }
        active.push_back(Active { iv.last, slot });
        Value temp = iv.value.withIndex(slot);
        temp.setLastUseOrdinal(iv.last);
        values.push_back(std::move(temp));
    }

    return values;
}

} // namespace codegen
