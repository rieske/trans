#include "FrameLayout.h"

#include <algorithm>
#include <limits>
#include <map>
#include <set>
#include <vector>

#include "SymbolRefs.h"
#include "types/ObjectAbi.h"

namespace codegen {
namespace {

constexpr int kPinnedLast = std::numeric_limits<int>::max();

int firstFit(const std::vector<int>& occ, int words, int first) {
    const int n = static_cast<int>(occ.size());
    for (int start = 0; start + words <= n; ++start) {
        int w = 0;
        while (w < words && occ[static_cast<std::size_t>(start + w)] < first) {
            ++w;
        }
        if (w == words) {
            return start;
        }
    }
    return -1;
}

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
        return !local.isExpressionTemp() || addressTaken.count(local.id()) != 0;
    };

    std::vector<Value> values;
    int pinnedWords = 0;
    for (const auto& local : locals) {
        if (!pinned(local)) {
            continue;
        }
        const int words = type::object_abi::valueWords(local.getSizeInBytes());
        Value home = local.withIndex(pinnedWords);
        if (local.isExpressionTemp()) {
            home.setLastUseOrdinal(liveRange(live, local.id()).second);
        }
        values.push_back(std::move(home));
        pinnedWords += words;
    }

    struct Interval {
        Value value;
        int words;
        int first;
        int last;
    };
    std::vector<Interval> intervals;
    for (auto& local : locals) {
        if (pinned(local)) {
            continue;
        }
        const auto range = liveRange(live, local.id());
        const int words = type::object_abi::valueWords(local.getSizeInBytes());
        intervals.push_back(Interval { std::move(local), words, range.first, range.second });
    }
    std::sort(intervals.begin(), intervals.end(),
            [](const Interval& a, const Interval& b) {
                if (a.first != b.first) {
                    return a.first < b.first;
                }
                return a.value.id() < b.value.id();
            });

    std::vector<int> occ(static_cast<std::size_t>(pinnedWords), kPinnedLast);
    for (auto& iv : intervals) {
        int slot = firstFit(occ, iv.words, iv.first);
        if (slot < 0) {
            slot = static_cast<int>(occ.size());
            occ.resize(static_cast<std::size_t>(slot + iv.words));
        }
        for (int w = 0; w < iv.words; ++w) {
            occ[static_cast<std::size_t>(slot + w)] = iv.last;
        }
        Value temp = iv.value.withIndex(slot);
        temp.setLastUseOrdinal(iv.last);
        values.push_back(std::move(temp));
    }

    return values;
}

} // namespace codegen
