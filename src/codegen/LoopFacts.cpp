#include "LoopFacts.h"

#include "Preheader.h"
#include "SymbolRefs.h"

namespace codegen {

LoopSnap analyzeLoops(const Cfg& cfg) {
    LoopSnap snap;
    snap.pred = cfgPredecessors(cfg);
    snap.dom = dominators(cfg, snap.pred);
    snap.loops = naturalLoops(cfg, snap.pred, snap.dom);
    return snap;
}

namespace {

bool isIndirectWriteOp(Op op) {
    return op == Op::LvalueAssign || op == Op::Call || op == Op::VaStart || op == Op::VaArg
            || op == Op::VaCopy || op == Op::VaEnd;
}

} // namespace

UseDefIndex buildUseDefIndex(const Cfg& cfg, Procedure& procedure) {
    UseDefIndex index;
    for (auto& value : procedure.frame.locals) {
        index.values[value.id()] = &value;
    }
    for (auto& value : procedure.frame.arguments) {
        index.values[value.id()] = &value;
    }
    index.indirectWrite.assign(cfg.size(), 0);
    for (std::size_t b = 0; b < cfg.size(); ++b) {
        for (const auto& inst : cfg[b].insts) {
            if (isIndirectWriteOp(inst.op)) {
                index.indirectWrite[b] = 1;
            }
            SymbolRefs refs;
            collectSymbolRefs(inst, refs);
            if (refs.addressOfBase != kNoSymbol) {
                index.addressTaken.insert(refs.addressOfBase);
            }
            for (int id : refs.defs) {
                ++index.defCount[id];
                index.defBlocks[id].insert(b);
            }
            for (int id : refs.uses) {
                index.useBlocks[id].insert(b);
            }
        }
    }
    return index;
}

Value* findValue(const UseDefIndex& index, int id) {
    const auto it = index.values.find(id);
    return it == index.values.end() ? nullptr : it->second;
}

bool defInLoop(const UseDefIndex& index, const NaturalLoop& loop, int id) {
    const auto it = index.defBlocks.find(id);
    if (it == index.defBlocks.end()) {
        return false;
    }
    for (const std::size_t b : it->second) {
        if (loop.blocks.count(b) != 0) {
            return true;
        }
    }
    return false;
}

bool loopHasIndirectWrite(const UseDefIndex& index, const NaturalLoop& loop) {
    for (const std::size_t b : loop.blocks) {
        if (b < index.indirectWrite.size() && index.indirectWrite[b]) {
            return true;
        }
    }
    return false;
}

void appendBeforeTerminator(BasicBlock& block, Instruction inst) {
    if (!block.insts.empty() && instructionTransfersControl(block.insts.back())) {
        block.insts.insert(block.insts.end() - 1, std::move(inst));
        return;
    }
    block.insts.push_back(std::move(inst));
}

int insertPreheadersFor(Cfg& cfg, IrStringTable& strings, std::unordered_set<int> need) {
    int inserted = 0;
    while (!need.empty()) {
        const LoopSnap snap = analyzeLoops(cfg);
        const NaturalLoop* chosen = nullptr;
        for (const auto& loop : snap.loops) {
            if (loop.header >= cfg.size() || need.count(cfg[loop.header].label) == 0) {
                continue;
            }
            if (chosen == nullptr || loop.header > chosen->header) {
                chosen = &loop;
            }
        }
        if (chosen == nullptr) {
            break;
        }
        const int headerLabel = cfg[chosen->header].label;
        if (hasPreheader(cfg, *chosen, snap.pred, snap.dom)) {
            need.erase(headerLabel);
            continue;
        }
        insertOne(cfg, *chosen, strings);
        need.erase(headerLabel);
        ++inserted;
    }
    return inserted;
}

} // namespace codegen
