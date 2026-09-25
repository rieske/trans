#include "Licm.h"

#include "Cfg.h"
#include "LoopFacts.h"
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

bool loopWritesAddressedObject(const Cfg& cfg, const NaturalLoop& loop, const UseDefIndex& index) {
    for (const std::size_t b : loop.blocks) {
        if (b >= cfg.size()) {
            continue;
        }
        for (const auto& inst : cfg[b].insts) {
            SymbolRefs refs;
            collectSymbolRefs(inst, refs);
            for (int id : refs.defs) {
                if (index.addressTaken.count(id) != 0 || findValue(index, id) == nullptr) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool loadCanBeSkipped(const Cfg& cfg, const NaturalLoop& loop, std::size_t loadBlock) {
    if (loadBlock == loop.header) {
        return false;
    }
    std::vector<char> seen(cfg.size(), 0);
    std::vector<std::size_t> stack { loop.header };
    if (loop.header < seen.size()) {
        seen[loop.header] = 1;
    }
    while (!stack.empty()) {
        const std::size_t b = stack.back();
        stack.pop_back();
        if (b == loadBlock) {
            continue;
        }
        for (const std::size_t succ : cfgSuccessors(cfg, b)) {
            if (loop.blocks.count(succ) == 0) {
                return true;
            }
            if (succ < seen.size() && seen[succ] == 0) {
                seen[succ] = 1;
                stack.push_back(succ);
            }
        }
    }
    return false;
}

bool isHoistable(const Cfg& cfg, const Instruction& inst, const NaturalLoop& loop,
        std::size_t block, const UseDefIndex& index, const std::unordered_set<int>& invariantTemps,
        bool indirectWrite, bool writesAddressed) {
    const bool load = inst.op == Op::Dereference;
    if (load && (indirectWrite || writesAddressed || loadCanBeSkipped(cfg, loop, block))) {
        return false;
    }
    if ((!isLicmPureOp(inst.op) && !load) || inst.result == kNoSymbol) {
        return false;
    }
    if ((inst.op == Op::FieldAddress || inst.op == Op::IndexAddress)
            && !symbols::addressBaseIsPointerValue(inst.baseMode)) {
        return false;
    }
    const Value* dest = findValue(index, inst.result);
    if (!dest || dest->isVolatile() || !dest->isExpressionTemp()
            || index.addressTaken.count(inst.result) != 0) {
        return false;
    }
    if (load) {
        const Value* address = findValue(index, inst.arg0);
        if (address && address->isVolatile()) {
            return false;
        }
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
    const bool writesAddressed = loopWritesAddressedObject(cfg, loop, index);
    for (const std::size_t b : loop.blocks) {
        if (b >= cfg.size()) {
            continue;
        }
        for (const auto& inst : cfg[b].insts) {
            if (isHoistable(cfg, inst, loop, b, index, none, indirectWrite, writesAddressed)) {
                return true;
            }
        }
    }
    return false;
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
    case Op::PointerAdd:
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
            need.insert(cfg[loop.header].label);
        }
    }
    stats.inserted = insertPreheadersFor(cfg, strings, std::move(need));

    if (stats.inserted != 0) {
        snap = analyzeLoops(cfg);
        index = buildUseDefIndex(cfg, procedure);
    }
    std::vector<int> pinned;
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
        const bool writesAddressed = loopWritesAddressedObject(cfg, loop, index);
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
                    if (!isHoistable(cfg, insts[i], loop, b, index, invariantTemps, indirectWrite,
                                writesAddressed)) {
                        ++i;
                        continue;
                    }
                    Instruction inst = insts[i];
                    insts.erase(insts.begin() + static_cast<std::ptrdiff_t>(i));
                    appendBeforeTerminator(cfg[*pre], inst);
                    invariantTemps.insert(inst.result);
                    pinned.push_back(inst.result);
                    ++stats.hoisted;
                    did = true;
                }
            }
        }
    }
    for (int id : pinned) {
        if (Value* dest = findValue(index, id)) {
            dest->clearExpressionTemp();
        }
    }
    if (stats.inserted != 0 || stats.hoisted != 0) {
        procedure.body = flattenCfg(cfg);
    }
    return stats;
}

} // namespace codegen
