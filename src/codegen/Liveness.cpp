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

} // namespace

LabelLiveIns computeLabelLiveIns(const Procedure& procedure) {
    LabelLiveIns out;
    const Cfg cfg = buildCfg(procedure.body);
    if (cfg.empty()) {
        return out;
    }
    const LinearPrep prep = prepare(procedure.body);
    const std::vector<BlockSets> sets = genKill(cfg, prep);

    std::vector<std::unordered_set<int>> liveIn(cfg.size());
    std::vector<std::unordered_set<int>> liveOut(cfg.size());
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = cfg.size(); i-- > 0;) {
            std::unordered_set<int> newOut;
            for (const std::size_t s : cfgSuccessors(cfg, i)) {
                newOut.insert(liveIn[s].begin(), liveIn[s].end());
            }
            std::unordered_set<int> newIn = sets[i].gen;
            for (int id : newOut) {
                if (sets[i].kill.count(id) == 0) {
                    newIn.insert(id);
                }
            }
            if (newIn != liveIn[i] || newOut != liveOut[i]) {
                liveIn[i] = std::move(newIn);
                liveOut[i] = std::move(newOut);
                changed = true;
            }
        }
    }

    for (std::size_t i = 0; i < cfg.size(); ++i) {
        if (cfg[i].label == kNoSymbol) {
            continue;
        }
        auto& dest = out.atLabel[cfg[i].label];
        dest = liveIn[i];
        dest.insert(prep.addressTaken.begin(), prep.addressTaken.end());
    }
    return out;
}

} // namespace codegen
