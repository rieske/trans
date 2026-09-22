#include "Loops.h"

#include <algorithm>
#include <cstdint>

namespace codegen {
namespace {

std::size_t wordCount(std::size_t n) {
    return (n + 63) / 64;
}

void setBit(DomBits& bits, std::size_t i) {
    bits.words[i / 64] |= std::uint64_t { 1 } << (i % 64);
}

DomBits bitsForAll(std::size_t n) {
    DomBits bits;
    bits.words.assign(wordCount(n), ~std::uint64_t { 0 });
    const std::size_t rem = n % 64;
    if (rem != 0) {
        bits.words.back() = (std::uint64_t { 1 } << rem) - 1;
    }
    return bits;
}

DomBits bitsForOne(std::size_t n, std::size_t i) {
    DomBits bits;
    bits.words.assign(wordCount(n), 0);
    setBit(bits, i);
    return bits;
}

void andEq(DomBits& dst, const DomBits& src) {
    for (std::size_t w = 0; w < dst.words.size(); ++w) {
        dst.words[w] &= src.words[w];
    }
}

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

bool hasBackwardJump(const std::vector<Instruction>& body) {
    std::unordered_set<int> seen;
    for (const auto& inst : body) {
        if (inst.op == Op::Label && inst.arg0 != kNoSymbol) {
            seen.insert(inst.arg0);
        } else if (inst.op == Op::Jump && inst.arg0 != kNoSymbol && seen.count(inst.arg0) != 0) {
            return true;
        }
    }
    return false;
}

std::vector<std::vector<std::size_t>> cfgPredecessors(const Cfg& cfg) {
    std::vector<std::vector<std::size_t>> pred(cfg.size());
    for (std::size_t i = 0; i < cfg.size(); ++i) {
        for (const std::size_t s : cfgSuccessors(cfg, i)) {
            pred[s].push_back(i);
        }
    }
    return pred;
}

std::vector<DomBits> dominators(const Cfg& cfg, const std::vector<std::vector<std::size_t>>& pred) {
    const std::size_t n = cfg.size();
    std::vector<DomBits> dom(n);
    if (n == 0) {
        return dom;
    }
    const DomBits all = bitsForAll(n);
    dom[0] = bitsForOne(n, 0);
    for (std::size_t i = 1; i < n; ++i) {
        dom[i] = all;
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = 1; i < n; ++i) {
            if (pred[i].empty()) {
                continue;
            }
            DomBits meet = dom[pred[i].front()];
            for (std::size_t p = 1; p < pred[i].size(); ++p) {
                andEq(meet, dom[pred[i][p]]);
            }
            setBit(meet, i);
            if (meet.words != dom[i].words) {
                dom[i] = std::move(meet);
                changed = true;
            }
        }
    }
    const auto reachable = reachableFromEntry(cfg);
    for (std::size_t i = 0; i < n; ++i) {
        if (!reachable[i]) {
            std::fill(dom[i].words.begin(), dom[i].words.end(), 0);
        }
    }
    return dom;
}

std::vector<DomBits> dominators(const Cfg& cfg) {
    return dominators(cfg, cfgPredecessors(cfg));
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

std::vector<NaturalLoop> naturalLoops(const Cfg& cfg,
        const std::vector<std::vector<std::size_t>>& pred, const std::vector<DomBits>& dom) {
    std::vector<NaturalLoop> byHeader(cfg.size());
    std::vector<char> seen(cfg.size(), 0);
    for (std::size_t n = 0; n < cfg.size(); ++n) {
        for (const std::size_t h : cfgSuccessors(cfg, n)) {
            if (!dom[n].test(h)) {
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

std::vector<NaturalLoop> naturalLoops(const Cfg& cfg) {
    return naturalLoops(cfg, cfgPredecessors(cfg), dominators(cfg));
}

std::optional<std::size_t> preheaderIndex(const Cfg& cfg, const NaturalLoop& loop,
        const std::vector<std::vector<std::size_t>>& pred, const std::vector<DomBits>& dom) {
    if (loop.header >= cfg.size()) {
        return std::nullopt;
    }
    std::unordered_set<std::size_t> latches(loop.latches.begin(), loop.latches.end());
    std::vector<std::size_t> nonLatch;
    std::unordered_set<std::size_t> seenPred;
    for (const std::size_t p : pred[loop.header]) {
        if (!seenPred.insert(p).second) {
            continue;
        }
        if (latches.count(p) != 0) {
            continue;
        }
        if (!dom[loop.header].test(p)) {
            continue;
        }
        nonLatch.push_back(p);
    }
    if (nonLatch.size() != 1) {
        return std::nullopt;
    }
    const std::size_t P = nonLatch.front();
    if (loop.blocks.count(P) != 0) {
        return std::nullopt;
    }
    std::unordered_set<std::size_t> succs;
    for (const std::size_t s : cfgSuccessors(cfg, P)) {
        succs.insert(s);
    }
    if (succs.size() != 1 || succs.count(loop.header) == 0) {
        return std::nullopt;
    }
    return P;
}

std::optional<std::size_t> preheaderIndex(const Cfg& cfg, const NaturalLoop& loop) {
    return preheaderIndex(cfg, loop, cfgPredecessors(cfg), dominators(cfg));
}

bool hasPreheader(const Cfg& cfg, const NaturalLoop& loop,
        const std::vector<std::vector<std::size_t>>& pred, const std::vector<DomBits>& dom) {
    return preheaderIndex(cfg, loop, pred, dom).has_value();
}

bool hasPreheader(const Cfg& cfg, const NaturalLoop& loop) {
    return preheaderIndex(cfg, loop).has_value();
}

} // namespace codegen
