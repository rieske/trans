#include "Liveness.h"

#include "Cfg.h"
#include "SymbolRefs.h"

namespace codegen {
namespace {

struct LinearPrep {
    std::vector<std::vector<int>> extraCallUses;
    std::unordered_set<int> addressTaken;
};

LinearPrep prepare(const std::vector<Instruction>& body) {
    LinearPrep prep;
    prep.extraCallUses.resize(body.size());
    std::vector<int> pending;
    for (std::size_t i = 0; i < body.size(); ++i) {
        SymbolRefs refs;
        collectSymbolRefs(body[i], refs);
        if (refs.addressOfBase != kNoSymbol) {
            prep.addressTaken.insert(refs.addressOfBase);
        }
        if (refs.isParam) {
            pending.insert(pending.end(), refs.uses.begin(), refs.uses.end());
        }
        if (refs.isCall) {
            prep.extraCallUses[i] = pending;
            pending.clear();
        }
    }
    return prep;
}

struct BlockSets {
    std::unordered_set<int> gen;
    std::unordered_set<int> kill;
};

std::vector<BlockSets> genKill(const Cfg& cfg, const LinearPrep& prep) {
    std::vector<BlockSets> sets(cfg.size());
    std::size_t bodyIndex = 0;
    for (std::size_t b = 0; b < cfg.size(); ++b) {
        if (cfg[b].label != kNoSymbol) {
            ++bodyIndex;
        }
        for (const auto& inst : cfg[b].insts) {
            SymbolRefs refs;
            collectSymbolRefs(inst, refs);
            if (refs.isCall && bodyIndex < prep.extraCallUses.size()) {
                for (int id : prep.extraCallUses[bodyIndex]) {
                    refs.addUse(id);
                }
            }
            for (int id : refs.uses) {
                if (sets[b].kill.count(id) == 0) {
                    sets[b].gen.insert(id);
                }
            }
            for (int id : refs.defs) {
                sets[b].kill.insert(id);
            }
            ++bodyIndex;
        }
    }
    return sets;
}

struct LiveSets {
    std::vector<std::unordered_set<int>> liveIn;
    std::vector<std::unordered_set<int>> liveOut;
};

LiveSets solveLiveness(const Cfg& cfg, const std::vector<BlockSets>& sets) {
    LiveSets live;
    live.liveIn.resize(cfg.size());
    live.liveOut.resize(cfg.size());
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = cfg.size(); i-- > 0;) {
            std::unordered_set<int> newOut;
            for (const std::size_t s : cfgSuccessors(cfg, i)) {
                newOut.insert(live.liveIn[s].begin(), live.liveIn[s].end());
            }
            std::unordered_set<int> newIn = sets[i].gen;
            for (int id : newOut) {
                if (sets[i].kill.count(id) == 0) {
                    newIn.insert(id);
                }
            }
            if (newIn != live.liveIn[i] || newOut != live.liveOut[i]) {
                live.liveIn[i] = std::move(newIn);
                live.liveOut[i] = std::move(newOut);
                changed = true;
            }
        }
    }
    return live;
}

void applyRefsBackward(std::unordered_set<int>& later, const SymbolRefs& refs) {
    for (int id : refs.defs) {
        later.erase(id);
    }
    later.insert(refs.uses.begin(), refs.uses.end());
}

} // namespace

ProcedureLiveness computeProcedureLiveness(const Procedure& procedure) {
    ProcedureLiveness out;
    const Cfg cfg = buildCfg(procedure.body);
    if (cfg.empty()) {
        return out;
    }
    const LinearPrep prep = prepare(procedure.body);
    out.addressTaken = prep.addressTaken;
    const LiveSets live = solveLiveness(cfg, genKill(cfg, prep));

    for (std::size_t i = 0; i < cfg.size(); ++i) {
        if (cfg[i].label == kNoSymbol) {
            continue;
        }
        auto& dest = out.atLabel[cfg[i].label];
        dest = live.liveIn[i];
        dest.insert(prep.addressTaken.begin(), prep.addressTaken.end());
    }

    std::size_t bodyIndex = 0;
    for (std::size_t b = 0; b < cfg.size(); ++b) {
        if (cfg[b].label != kNoSymbol) {
            ++bodyIndex;
        }
        std::vector<std::size_t> instIndex;
        instIndex.reserve(cfg[b].insts.size());
        for (std::size_t n = 0; n < cfg[b].insts.size(); ++n) {
            instIndex.push_back(bodyIndex++);
        }
        std::unordered_set<int> later = live.liveOut[b];
        for (std::size_t n = cfg[b].insts.size(); n-- > 0;) {
            SymbolRefs refs;
            collectSymbolRefs(cfg[b].insts[n], refs);
            if (refs.isCall && instIndex[n] < prep.extraCallUses.size()) {
                out.afterCall[static_cast<int>(instIndex[n])] = later;
                for (int id : prep.extraCallUses[instIndex[n]]) {
                    refs.addUse(id);
                }
            } else if (refs.isCall) {
                out.afterCall[static_cast<int>(instIndex[n])] = later;
            }
            applyRefsBackward(later, refs);
        }
    }
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
