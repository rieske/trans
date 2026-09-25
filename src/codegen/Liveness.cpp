#include "Liveness.h"

#include "Cfg.h"
#include "SymbolRefs.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <vector>

namespace codegen {
namespace {

struct LinearPrep {
    std::vector<std::vector<int>> extraCallUses;
    std::unordered_set<int> addressTaken;
    std::unordered_map<int, int> dense;
    std::vector<int> ids;

    int denseId(int id) const {
        const auto it = dense.find(id);
        return it == dense.end() ? -1 : it->second;
    }
};

LinearPrep prepare(const std::vector<Instruction>& body) {
    LinearPrep prep;
    prep.extraCallUses.resize(body.size());
    auto number = [&](int id) {
        const auto [it, added] = prep.dense.emplace(id, static_cast<int>(prep.ids.size()));
        if (added) {
            prep.ids.push_back(id);
        }
        return it->second;
    };
    std::vector<int> pending;
    for (std::size_t i = 0; i < body.size(); ++i) {
        SymbolRefs refs;
        collectSymbolRefs(body[i], refs);
        if (refs.addressOfBase != kNoSymbol) {
            prep.addressTaken.insert(refs.addressOfBase);
        }
        for (int id : refs.defs) {
            number(id);
        }
        for (int id : refs.uses) {
            const int d = number(id);
            if (refs.isParam) {
                pending.push_back(d);
            }
        }
        if (refs.isCall) {
            prep.extraCallUses[i] = pending;
            pending.clear();
        }
    }
    return prep;
}

struct Bits {
    std::vector<std::uint64_t> words;

    explicit Bits(std::size_t width) : words((width + 63) / 64, 0) {}

    bool inRange(int id) const {
        return id >= 0 && (static_cast<std::size_t>(id) >> 6) < words.size();
    }

    void set(int id) {
        if (inRange(id)) {
            words[static_cast<std::size_t>(id) >> 6] |= 1ull << (id & 63);
        }
    }

    void clear(int id) {
        if (inRange(id)) {
            words[static_cast<std::size_t>(id) >> 6] &= ~(1ull << (id & 63));
        }
    }

    bool test(int id) const {
        if (!inRange(id)) {
            return false;
        }
        return (words[static_cast<std::size_t>(id) >> 6] >> (id & 63)) & 1ull;
    }

    bool operator==(const Bits& other) const { return words == other.words; }
};

struct BlockSets {
    Bits gen;
    Bits kill;
};

std::vector<BlockSets> genKill(const Cfg& cfg, const LinearPrep& prep, std::size_t width) {
    std::vector<BlockSets> sets;
    sets.reserve(cfg.size());
    for (std::size_t b = 0; b < cfg.size(); ++b) {
        sets.push_back(BlockSets { Bits(width), Bits(width) });
    }
    std::size_t bodyIndex = 0;
    for (std::size_t b = 0; b < cfg.size(); ++b) {
        if (cfg[b].label != kNoSymbol) {
            ++bodyIndex;
        }
        for (const auto& inst : cfg[b].insts) {
            SymbolRefs refs;
            collectSymbolRefs(inst, refs);
            if (refs.isCall) {
                for (int d : prep.extraCallUses[bodyIndex]) {
                    if (!sets[b].kill.test(d)) {
                        sets[b].gen.set(d);
                    }
                }
            }
            for (int id : refs.uses) {
                const int d = prep.denseId(id);
                if (!sets[b].kill.test(d)) {
                    sets[b].gen.set(d);
                }
            }
            for (int id : refs.defs) {
                sets[b].kill.set(prep.denseId(id));
            }
            ++bodyIndex;
        }
    }
    return sets;
}

struct LiveBits {
    std::vector<Bits> liveIn;
    std::vector<Bits> liveOut;
};

LiveBits solveLiveness(const Cfg& cfg, const std::vector<BlockSets>& sets, std::size_t width) {
    LiveBits live;
    live.liveIn.assign(cfg.size(), Bits(width));
    live.liveOut.assign(cfg.size(), Bits(width));
    std::vector<std::vector<std::size_t>> succ(cfg.size());
    for (std::size_t i = 0; i < cfg.size(); ++i) {
        succ[i] = cfgSuccessors(cfg, i);
    }
    Bits newOut(width);
    Bits newIn(width);
    const std::size_t words = newOut.words.size();
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = cfg.size(); i-- > 0;) {
            std::fill(newOut.words.begin(), newOut.words.end(), 0);
            for (const std::size_t s : succ[i]) {
                for (std::size_t w = 0; w < words; ++w) {
                    newOut.words[w] |= live.liveIn[s].words[w];
                }
            }
            for (std::size_t w = 0; w < words; ++w) {
                newIn.words[w] = sets[i].gen.words[w] | (newOut.words[w] & ~sets[i].kill.words[w]);
            }
            if (newIn == live.liveIn[i] && newOut == live.liveOut[i]) {
                continue;
            }
            live.liveIn[i].words = newIn.words;
            live.liveOut[i].words = newOut.words;
            changed = true;
        }
    }
    return live;
}

std::unordered_set<int> toSet(const Bits& bits, const std::vector<int>& ids) {
    std::unordered_set<int> out;
    for (std::size_t w = 0; w < bits.words.size(); ++w) {
        std::uint64_t word = bits.words[w];
        while (word != 0) {
            out.insert(ids[w * 64 + static_cast<std::size_t>(std::countr_zero(word))]);
            word &= word - 1;
        }
    }
    return out;
}

struct Solved {
    Cfg cfg;
    LinearPrep prep;
    LiveBits live;
};

Solved solve(const Procedure& procedure) {
    Solved solved;
    solved.cfg = buildCfg(procedure.body);
    solved.prep = prepare(procedure.body);
    const std::size_t width = solved.prep.ids.size();
    solved.live = solveLiveness(solved.cfg, genKill(solved.cfg, solved.prep, width), width);
    return solved;
}

template <typename Visit>
void walkBackward(const Solved& solved, Visit visit) {
    const Cfg& cfg = solved.cfg;
    const LinearPrep& prep = solved.prep;
    std::size_t bodyIndex = 0;
    for (std::size_t b = 0; b < cfg.size(); ++b) {
        if (cfg[b].label != kNoSymbol) {
            ++bodyIndex;
        }
        const std::size_t start = bodyIndex;
        bodyIndex += cfg[b].insts.size();
        Bits later = solved.live.liveOut[b];
        for (std::size_t n = cfg[b].insts.size(); n-- > 0;) {
            const std::size_t index = start + n;
            SymbolRefs refs;
            collectSymbolRefs(cfg[b].insts[n], refs);
            visit(cfg[b].insts[n], index, refs, later);
            for (int id : refs.defs) {
                later.clear(prep.denseId(id));
            }
            for (int id : refs.uses) {
                later.set(prep.denseId(id));
            }
            if (refs.isCall) {
                for (int d : prep.extraCallUses[index]) {
                    later.set(d);
                }
            }
        }
    }
}

} // namespace

ProcedureLiveness computeProcedureLiveness(const Procedure& procedure) {
    ProcedureLiveness out;
    const Solved solved = solve(procedure);
    out.addressTaken = solved.prep.addressTaken;
    for (std::size_t b = 0; b < solved.cfg.size(); ++b) {
        if (solved.cfg[b].label == kNoSymbol) {
            continue;
        }
        auto& dest = out.atLabel[solved.cfg[b].label];
        dest = toSet(solved.live.liveIn[b], solved.prep.ids);
        dest.insert(solved.prep.addressTaken.begin(), solved.prep.addressTaken.end());
    }
    walkBackward(solved, [&](const Instruction&, std::size_t index, const SymbolRefs& refs, const Bits& later) {
        if (refs.isCall) {
            out.afterCall[static_cast<int>(index)] = toSet(later, solved.prep.ids);
        }
    });
    return out;
}

TempLiveness computeTempLiveness(const Procedure& procedure) {
    TempLiveness out;
    const Solved solved = solve(procedure);
    out.resultLiveAfter.assign(procedure.body.size(), 0);
    out.addressTaken = solved.prep.addressTaken;
    walkBackward(solved, [&](const Instruction& inst, std::size_t index, const SymbolRefs&, const Bits& later) {
        if (later.test(solved.prep.denseId(inst.result))) {
            out.resultLiveAfter[index] = 1;
        }
    });
    return out;
}

LabelLiveIns computeLabelLiveIns(const Procedure& procedure) {
    LabelLiveIns out;
    out.atLabel = std::move(computeProcedureLiveness(procedure).atLabel);
    return out;
}

std::unordered_map<int, std::unordered_set<int>> computeLiveAfterCalls(const Procedure& procedure) {
    return computeProcedureLiveness(procedure).afterCall;
}

} // namespace codegen
