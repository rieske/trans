#include "Loops.h"

#include <algorithm>

namespace codegen {
namespace {

std::vector<char> reachableFromEntry(const Cfg& cfg) {
    std::vector<char> reachable(cfg.size(), 0);
    if (cfg.empty()) {
        return reachable;
    }
    std::vector<std::size_t> work { 0 };
    reachable[0] = 1;
    while (!work.empty()) {
        const std::size_t i = work.back();
        work.pop_back();
        for (const std::size_t s : cfgSuccessors(cfg, i)) {
            if (!reachable[s]) {
                reachable[s] = 1;
                work.push_back(s);
            }
        }
    }
    return reachable;
}

} // namespace

std::vector<std::vector<std::size_t>> cfgPredecessors(const Cfg& cfg) {
    std::vector<std::vector<std::size_t>> pred(cfg.size());
    for (std::size_t i = 0; i < cfg.size(); ++i) {
        for (const std::size_t s : cfgSuccessors(cfg, i)) {
            pred[s].push_back(i);
        }
    }
    return pred;
}

std::vector<std::unordered_set<std::size_t>> dominators(const Cfg& cfg) {
    const std::size_t n = cfg.size();
    std::vector<std::unordered_set<std::size_t>> dom(n);
    if (n == 0) {
        return dom;
    }
    std::unordered_set<std::size_t> all;
    for (std::size_t i = 0; i < n; ++i) {
        all.insert(i);
    }
    dom[0] = { 0 };
    for (std::size_t i = 1; i < n; ++i) {
        dom[i] = all;
    }
    const auto pred = cfgPredecessors(cfg);
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = 1; i < n; ++i) {
            if (pred[i].empty()) {
                continue;
            }
            std::unordered_set<std::size_t> meet = dom[pred[i].front()];
            for (std::size_t p = 1; p < pred[i].size(); ++p) {
                std::unordered_set<std::size_t> next;
                for (const std::size_t b : meet) {
                    if (dom[pred[i][p]].count(b) != 0) {
                        next.insert(b);
                    }
                }
                meet = std::move(next);
            }
            meet.insert(i);
            if (meet != dom[i]) {
                dom[i] = std::move(meet);
                changed = true;
            }
        }
    }
    const auto reachable = reachableFromEntry(cfg);
    for (std::size_t i = 0; i < n; ++i) {
        if (!reachable[i]) {
            dom[i].clear();
        }
    }
    return dom;
}

namespace {

std::unordered_set<std::size_t> loopBlocks(std::size_t header, std::size_t latch,
        const std::vector<std::vector<std::size_t>>& pred) {
    std::unordered_set<std::size_t> blocks { header };
    if (latch == header) {
        return blocks;
    }
    std::vector<std::size_t> work { latch };
    blocks.insert(latch);
    while (!work.empty()) {
        const std::size_t x = work.back();
        work.pop_back();
        for (const std::size_t p : pred[x]) {
            if (blocks.insert(p).second) {
                work.push_back(p);
            }
        }
    }
    return blocks;
}

} // namespace

std::vector<NaturalLoop> naturalLoops(const Cfg& cfg) {
    const auto pred = cfgPredecessors(cfg);
    const auto dom = dominators(cfg);
    std::vector<NaturalLoop> byHeader(cfg.size());
    std::vector<char> seen(cfg.size(), 0);
    for (std::size_t n = 0; n < cfg.size(); ++n) {
        for (const std::size_t h : cfgSuccessors(cfg, n)) {
            if (dom[n].count(h) == 0) {
                continue;
            }
            seen[h] = 1;
            NaturalLoop& loop = byHeader[h];
            loop.header = h;
            loop.latches.push_back(n);
            const auto extra = loopBlocks(h, n, pred);
            loop.blocks.insert(extra.begin(), extra.end());
        }
    }
    std::vector<NaturalLoop> out;
    for (std::size_t h = 0; h < cfg.size(); ++h) {
        if (!seen[h]) {
            continue;
        }
        std::sort(byHeader[h].latches.begin(), byHeader[h].latches.end());
        out.push_back(std::move(byHeader[h]));
    }
    return out;
}

} // namespace codegen
