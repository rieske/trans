#include "Licm.h"

#include "Cfg.h"
#include "Loops.h"
#include "Preheader.h"
#include "SymbolRefs.h"
#include "symbols/AddressPlan.h"

#include <algorithm>
#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace codegen {
namespace {

struct UseDefIndex {
    std::unordered_map<int, Value*> values;
    std::unordered_map<int, int> defCount;
    std::unordered_map<int, std::unordered_set<std::size_t>> defBlocks;
    std::unordered_map<int, std::unordered_set<std::size_t>> useBlocks;
    std::unordered_set<int> addressTaken;
    std::vector<char> indirectWrite;
};

bool isIndirectWriteOp(Op op) {
    return op == Op::LvalueAssign || op == Op::Call || op == Op::VaStart || op == Op::VaArg
            || op == Op::VaCopy || op == Op::VaEnd;
}

UseDefIndex buildUseDefIndex(Cfg& cfg, Procedure& procedure) {
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

bool usesOutsideLoop(const UseDefIndex& index, const NaturalLoop& loop, int id) {
    const auto it = index.useBlocks.find(id);
    if (it == index.useBlocks.end()) {
        return false;
    }
    for (const std::size_t b : it->second) {
        if (loop.blocks.count(b) == 0) {
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

bool operandsInvariant(const Instruction& inst, const NaturalLoop& loop, const UseDefIndex& index,
        const std::unordered_set<int>& invariantTemps, bool indirectWrite) {
    if (inst.op == Op::AssignConstant || inst.op == Op::AssignLabelAddress
            || inst.op == Op::FunctionAddress) {
        return true;
    }
    SymbolRefs refs;
    collectSymbolRefs(inst, refs);
    for (int id : refs.uses) {
        if (id == kNoSymbol) {
            continue;
        }
        const Value* value = findValue(index, id);
        if (!value || index.addressTaken.count(id) != 0) {
            return false;
        }
        const bool outsideOrInvariant =
                !defInLoop(index, loop, id) || invariantTemps.count(id) != 0;
        if (!outsideOrInvariant) {
            return false;
        }
        if (indirectWrite && !value->isExpressionTemp()) {
            return false;
        }
    }
    return true;
}

bool isHoistable(const Instruction& inst, const NaturalLoop& loop, const UseDefIndex& index,
        const std::unordered_set<int>& invariantTemps, bool indirectWrite) {
    if (!isLicmPureOp(inst.op) || inst.result == kNoSymbol) {
        return false;
    }
    if ((inst.op == Op::FieldAddress || inst.op == Op::IndexAddress)
            && !symbols::addressBaseIsPointerValue(inst.baseMode)) {
        return false;
    }
    const Value* dest = findValue(index, inst.result);
    if (!dest || !dest->isExpressionTemp() || index.addressTaken.count(inst.result) != 0) {
        return false;
    }
    const auto defs = index.defCount.find(inst.result);
    if (defs == index.defCount.end() || defs->second != 1) {
        return false;
    }
    if (usesOutsideLoop(index, loop, inst.result)) {
        return false;
    }
    return operandsInvariant(inst, loop, index, invariantTemps, indirectWrite);
}

bool anyHoistable(const Cfg& cfg, const NaturalLoop& loop, const UseDefIndex& index) {
    const std::unordered_set<int> none;
    const bool indirectWrite = loopHasIndirectWrite(index, loop);
    for (const std::size_t b : loop.blocks) {
        if (b >= cfg.size()) {
            continue;
        }
        for (const auto& inst : cfg[b].insts) {
            if (isHoistable(inst, loop, index, none, indirectWrite)) {
                return true;
            }
        }
    }
    return false;
}

void appendBeforeTerminator(BasicBlock& block, Instruction inst) {
    if (!block.insts.empty() && instructionTransfersControl(block.insts.back())) {
        block.insts.insert(block.insts.end() - 1, std::move(inst));
    } else {
        block.insts.push_back(std::move(inst));
    }
}

int headerLabelOf(const Cfg& cfg, const NaturalLoop& loop) {
    if (loop.header >= cfg.size()) {
        return kNoSymbol;
    }
    return cfg[loop.header].label;
}

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

struct LoopSnap {
    std::vector<std::vector<std::size_t>> pred;
    std::vector<DomBits> dom;
    std::vector<NaturalLoop> loops;
};

LoopSnap analyzeLoops(const Cfg& cfg) {
    LoopSnap snap;
    snap.pred = cfgPredecessors(cfg);
    snap.dom = dominators(cfg, snap.pred);
    snap.loops = naturalLoops(cfg, snap.pred, snap.dom);
    return snap;
}

} // namespace

bool isLicmPureOp(Op op) {
    switch (op) {
    case Op::Add:
    case Op::Sub:
    case Op::Mul:
    case Op::And:
    case Op::Or:
    case Op::Xor:
    case Op::UnaryMinus:
    case Op::UnaryNot:
    case Op::Assign:
    case Op::AssignConstant:
    case Op::AssignLabelAddress:
    case Op::FunctionAddress:
    case Op::Widen:
    case Op::Bswap:
    case Op::Ctz:
    case Op::PointerOffset:
    case Op::PointerDiff:
    case Op::FieldAddress:
    case Op::IndexAddress:
        return true;
    default:
        return false;
    }
}

LicmStats hoistLoopInvariants(Procedure& procedure, IrStringTable& strings) {
    LicmStats stats;
    if (!hasBackwardJump(procedure.body)) {
        return stats;
    }
    Cfg cfg = buildCfg(procedure.body);
    UseDefIndex index = buildUseDefIndex(cfg, procedure);

    std::unordered_set<int> need;
    LoopSnap snap = analyzeLoops(cfg);
    for (const auto& loop : snap.loops) {
        if (loop.header >= cfg.size()) {
            continue;
        }
        if (cfg[loop.header].label == kNoSymbol && loop.header != 0) {
            continue;
        }
        if (anyHoistable(cfg, loop, index) && !hasPreheader(cfg, loop, snap.pred, snap.dom)) {
            need.insert(headerLabelOf(cfg, loop));
        }
    }
    while (!need.empty()) {
        snap = analyzeLoops(cfg);
        const NaturalLoop* chosen = nullptr;
        for (const auto& loop : snap.loops) {
            if (need.count(headerLabelOf(cfg, loop)) == 0) {
                continue;
            }
            if (!chosen || loop.header > chosen->header) {
                chosen = &loop;
            }
        }
        if (!chosen) {
            break;
        }
        const int headerLabel = headerLabelOf(cfg, *chosen);
        if (hasPreheader(cfg, *chosen, snap.pred, snap.dom)) {
            need.erase(headerLabel);
            continue;
        }
        insertOne(cfg, *chosen, strings);
        ++stats.inserted;
        need.erase(headerLabel);
    }

    if (stats.inserted != 0) {
        snap = analyzeLoops(cfg);
        index = buildUseDefIndex(cfg, procedure);
    }
    std::sort(snap.loops.begin(), snap.loops.end(), [](const NaturalLoop& a, const NaturalLoop& b) {
        if (a.blocks.size() != b.blocks.size()) {
            return a.blocks.size() < b.blocks.size();
        }
        return a.header < b.header;
    });
    for (const auto& loop : snap.loops) {
        ++stats.loopsVisited;
        const auto pre = preheaderIndex(cfg, loop, snap.pred, snap.dom);
        if (!pre) {
            continue;
        }
        const bool indirectWrite = loopHasIndirectWrite(index, loop);
        std::unordered_set<int> invariantTemps;
        bool did = true;
        while (did) {
            did = false;
            std::vector<std::size_t> blocks(loop.blocks.begin(), loop.blocks.end());
            std::sort(blocks.begin(), blocks.end());
            for (const std::size_t b : blocks) {
                if (b >= cfg.size()) {
                    continue;
                }
                auto& insts = cfg[b].insts;
                for (std::size_t i = 0; i < insts.size();) {
                    if (!isHoistable(insts[i], loop, index, invariantTemps, indirectWrite)) {
                        ++i;
                        continue;
                    }
                    Instruction inst = insts[i];
                    insts.erase(insts.begin() + static_cast<std::ptrdiff_t>(i));
                    if (Value* dest = findValue(index, inst.result)) {
                        dest->clearExpressionTemp();
                    }
                    appendBeforeTerminator(cfg[*pre], inst);
                    invariantTemps.insert(inst.result);
                    ++stats.hoisted;
                    did = true;
                }
            }
        }
    }
    if (stats.inserted != 0 || stats.hoisted != 0) {
        procedure.body = flattenCfg(cfg);
    }
    return stats;
}

} // namespace codegen
