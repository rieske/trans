#include "StrengthReduce.h"

#include "Cfg.h"
#include "IrBuilders.h"
#include "LoopFacts.h"
#include "Loops.h"
#include "Preheader.h"
#include "SymbolRefs.h"
#include "Value.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace codegen {
namespace {

struct Step {
    bool immediate { false };
    bool subtract { false };
    int imm { 0 };
    int value { kNoSymbol };
};

struct Update {
    std::size_t block { 0 };
    std::size_t index { 0 };
    Step step;
};

struct MulSite {
    std::size_t block { 0 };
    std::size_t index { 0 };
    int result { kNoSymbol };
};

struct Shape {
    Type type { Type::INTEGRAL };
    int size { 4 };
    type::sysv::Classification classification {};
};

struct Group {
    int iv { kNoSymbol };
    int factor { kNoSymbol };
    Update update;
    Shape shape;
    std::vector<MulSite> sites;
};

bool isIntegral(const UseDefIndex& index, int id) {
    const Value* value = findValue(index, id);
    return value != nullptr && value->getType() == Type::INTEGRAL;
}

bool isInvariant(const UseDefIndex& index, const NaturalLoop& loop, int id, bool indirect) {
    if (defInLoop(index, loop, id)) {
        return false;
    }
    if (indirect && index.addressTaken.count(id) != 0) {
        return false;
    }
    const Value* value = findValue(index, id);
    return value != nullptr && value->getType() == Type::INTEGRAL;
}

bool immediateStep(int raw, bool subtract, Step& step) {
    if (raw == std::numeric_limits<int>::min()) {
        return false;
    }
    step.immediate = true;
    if (raw < 0) {
        step.subtract = !subtract;
        step.imm = -raw;
    } else {
        step.subtract = subtract;
        step.imm = raw;
    }
    return true;
}

bool simpleStep(const Instruction& inst, int iv, Step& step) {
    if (inst.op == Op::Inc && inst.arg0 == iv) {
        return immediateStep(inst.imm, false, step);
    }
    if (inst.op == Op::Dec && inst.arg0 == iv) {
        return immediateStep(inst.imm, true, step);
    }
    if (inst.op == Op::Add && inst.result == iv) {
        if (inst.arg0 == iv && inst.arg1 != iv) {
            step.value = inst.arg1;
            return true;
        }
        if (inst.arg1 == iv && inst.arg0 != iv) {
            step.value = inst.arg0;
            return true;
        }
        return false;
    }
    if (inst.op == Op::Sub && inst.result == iv && inst.arg0 == iv && inst.arg1 != iv) {
        step.subtract = true;
        step.value = inst.arg1;
        return true;
    }
    return false;
}

bool dominatesLatches(const LoopSnap& snap, const NaturalLoop& loop, std::size_t block) {
    if (loop.latches.empty() || block >= snap.dom.size()) {
        return false;
    }
    for (const std::size_t latch : loop.latches) {
        if (latch >= snap.dom.size() || !snap.dom[latch].test(block)) {
            return false;
        }
    }
    return true;
}

bool ownedByLoop(const NaturalLoop& loop, std::size_t block, const std::vector<NaturalLoop>& loops) {
    if (loop.blocks.count(block) == 0) {
        return false;
    }
    for (const auto& other : loops) {
        if (other.blocks.count(block) != 0 && other.blocks.size() < loop.blocks.size()) {
            return false;
        }
    }
    return true;
}

std::unordered_map<int, Update> simpleIvs(const Cfg& cfg, const LoopSnap& snap, const UseDefIndex& index,
        const NaturalLoop& loop) {
    const bool indirect = loopHasIndirectWrite(index, loop);
    std::unordered_map<int, Update> updates;
    std::unordered_set<int> disqualified;
    for (const std::size_t b : loop.blocks) {
        if (b >= cfg.size()) {
            continue;
        }
        const auto& insts = cfg[b].insts;
        for (std::size_t i = 0; i < insts.size(); ++i) {
            SymbolRefs refs;
            collectSymbolRefs(insts[i], refs);
            for (int id : refs.defs) {
                if (id == kNoSymbol || !isIntegral(index, id)) {
                    continue;
                }
                Step step;
                if (!simpleStep(insts[i], id, step)) {
                    disqualified.insert(id);
                    continue;
                }
                if (updates.count(id) != 0) {
                    disqualified.insert(id);
                    continue;
                }
                updates.insert({ id, Update { b, i, step } });
            }
        }
    }
    std::unordered_map<int, Update> simple;
    for (const auto& entry : updates) {
        const int iv = entry.first;
        const Update& update = entry.second;
        if (disqualified.count(iv) != 0 || !dominatesLatches(snap, loop, update.block)
                || !ownedByLoop(loop, update.block, snap.loops)) {
            continue;
        }
        if (indirect && index.addressTaken.count(iv) != 0) {
            continue;
        }
        if (!update.step.immediate
                && !isInvariant(index, loop, update.step.value, indirect)) {
            continue;
        }
        simple.insert(entry);
    }
    return simple;
}

std::vector<Group> reducibleMuls(const Cfg& cfg, const LoopSnap& snap, const UseDefIndex& index,
        const NaturalLoop& loop) {
    const auto ivs = simpleIvs(cfg, snap, index, loop);
    const bool indirect = loopHasIndirectWrite(index, loop);
    std::vector<Group> groups;
    for (const std::size_t b : loop.blocks) {
        if (b >= cfg.size()) {
            continue;
        }
        const auto& insts = cfg[b].insts;
        for (std::size_t i = 0; i < insts.size(); ++i) {
            const Instruction& inst = insts[i];
            if (inst.op != Op::Mul) {
                continue;
            }
            int iv = kNoSymbol;
            int factor = kNoSymbol;
            if (ivs.count(inst.arg0) != 0 && isInvariant(index, loop, inst.arg1, indirect)) {
                iv = inst.arg0;
                factor = inst.arg1;
            } else if (ivs.count(inst.arg1) != 0 && isInvariant(index, loop, inst.arg0, indirect)) {
                iv = inst.arg1;
                factor = inst.arg0;
            } else {
                continue;
            }
            const Value* result = findValue(index, inst.result);
            if (result == nullptr || result->getType() != Type::INTEGRAL) {
                continue;
            }
            Group* group = nullptr;
            for (auto& existing : groups) {
                if (existing.iv == iv && existing.factor == factor) {
                    group = &existing;
                    break;
                }
            }
            if (group == nullptr) {
                Group created;
                created.iv = iv;
                created.factor = factor;
                created.update = ivs.at(iv);
                created.shape = Shape { result->getType(), result->getSizeInBytes(),
                        result->getClassification() };
                groups.push_back(std::move(created));
                group = &groups.back();
            }
            group->sites.push_back(MulSite { b, i, inst.result });
        }
    }
    return groups;
}

struct AddressSite {
    std::size_t block { 0 };
    std::size_t index { 0 };
    int result { kNoSymbol };
};

struct AddressGroup {
    int base { kNoSymbol };
    int iv { kNoSymbol };
    int stride { 0 };
    bool pointerForm { false };
    bool pointerSubtract { false };
    symbols::AddressBaseMode baseMode { symbols::AddressBaseMode::PointerValue };
    Update update;
    int byteDelta { 0 };
    Shape shape;
    std::vector<AddressSite> sites;
};

bool baseStable(const UseDefIndex& index, const NaturalLoop& loop, int id, bool indirect) {
    if (!isIntegral(index, id) || defInLoop(index, loop, id)) {
        return false;
    }
    return !(indirect && index.addressTaken.count(id) != 0);
}

bool rawRegisterIv(const UseDefIndex& index, int id) {
    const Value* value = findValue(index, id);
    return value != nullptr && value->getType() == Type::INTEGRAL && value->getSizeInBytes() == 8
            && value->getClassification().gprExtend == type::sysv::GprExtend::None;
}

bool constantByteDelta(const Step& step, int stride, bool pointerSubtract, int& bytes) {
    if (!step.immediate || stride < 0) {
        return false;
    }
    std::int64_t delta = std::int64_t { step.imm } * stride;
    if (step.subtract) {
        delta = -delta;
    }
    if (pointerSubtract) {
        delta = -delta;
    }
    if (delta > std::numeric_limits<int>::max() || delta < std::numeric_limits<int>::min()) {
        return false;
    }
    bytes = static_cast<int>(delta);
    return true;
}

std::vector<AddressGroup> reducibleAddresses(const Cfg& cfg, const LoopSnap& snap, const UseDefIndex& index,
        const NaturalLoop& loop) {
    const auto ivs = simpleIvs(cfg, snap, index, loop);
    const bool indirect = loopHasIndirectWrite(index, loop);
    std::vector<AddressGroup> groups;
    for (const std::size_t b : loop.blocks) {
        if (b >= cfg.size()) {
            continue;
        }
        const auto& insts = cfg[b].insts;
        for (std::size_t i = 0; i < insts.size(); ++i) {
            const Instruction& inst = insts[i];
            int base = kNoSymbol;
            int iv = kNoSymbol;
            int stride = 0;
            bool pointerForm = false;
            bool pointerSubtract = false;
            symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::PointerValue;
            if (inst.op == Op::IndexAddress) {
                base = inst.arg0;
                iv = inst.arg1;
                stride = inst.imm;
                baseMode = inst.baseMode;
            } else if (inst.op == Op::PointerOffset) {
                base = inst.arg0;
                iv = inst.arg1;
                stride = inst.imm;
                pointerForm = true;
                pointerSubtract = inst.pointerSubtract;
            } else {
                continue;
            }
            const auto ivIt = ivs.find(iv);
            if (ivIt == ivs.end() || !rawRegisterIv(index, iv) || !baseStable(index, loop, base, indirect)) {
                continue;
            }
            int byteDelta = 0;
            if (!constantByteDelta(ivIt->second.step, stride, pointerSubtract, byteDelta)) {
                continue;
            }
            const Value* result = findValue(index, inst.result);
            if (result == nullptr || result->getType() != Type::INTEGRAL) {
                continue;
            }
            AddressGroup* group = nullptr;
            for (auto& existing : groups) {
                if (existing.base == base && existing.iv == iv && existing.stride == stride
                        && existing.pointerForm == pointerForm
                        && existing.pointerSubtract == pointerSubtract
                        && existing.baseMode == baseMode) {
                    group = &existing;
                    break;
                }
            }
            if (group == nullptr) {
                AddressGroup created;
                created.base = base;
                created.iv = iv;
                created.stride = stride;
                created.pointerForm = pointerForm;
                created.pointerSubtract = pointerSubtract;
                created.baseMode = baseMode;
                created.update = ivIt->second;
                created.byteDelta = byteDelta;
                created.shape = Shape { result->getType(), result->getSizeInBytes(),
                        result->getClassification() };
                groups.push_back(std::move(created));
                group = &groups.back();
            }
            group->sites.push_back(AddressSite { b, i, inst.result });
        }
    }
    return groups;
}

bool loopNeedsPreheader(const Cfg& cfg, const LoopSnap& snap, const UseDefIndex& index,
        const NaturalLoop& loop) {
    if (hasPreheader(cfg, loop, snap.pred, snap.dom)) {
        return false;
    }
    if (loop.header >= cfg.size()) {
        return false;
    }
    if (cfg[loop.header].label == kNoSymbol && loop.header != 0) {
        return false;
    }
    return !reducibleMuls(cfg, snap, index, loop).empty()
            || !reducibleAddresses(cfg, snap, index, loop).empty();
}

int pinTemp(Procedure& procedure, IrStringTable& strings, const Shape& shape) {
    Value value { strings.internFresh("$sr"), 0, shape.type, shape.size, shape.classification };
    const int id = value.id();
    procedure.frame.locals.push_back(std::move(value));
    return id;
}

struct LatchEdit {
    std::size_t block { 0 };
    std::size_t index { 0 };
    Instruction inst;
};

void insertLatchEdits(Cfg& cfg, std::vector<LatchEdit> edits) {
    std::sort(edits.begin(), edits.end(), [](const LatchEdit& a, const LatchEdit& b) {
        if (a.block != b.block) {
            return a.block > b.block;
        }
        return a.index > b.index;
    });
    for (const LatchEdit& edit : edits) {
        auto& insts = cfg[edit.block].insts;
        insts.insert(insts.begin() + static_cast<std::ptrdiff_t>(edit.index + 1), edit.inst);
    }
}

void rewriteGroups(Cfg& cfg, Procedure& procedure, IrStringTable& strings, const LoopSnap& snap,
        const NaturalLoop& loop, const std::vector<Group>& groups, StrengthReduceStats& stats) {
    const auto pre = preheaderIndex(cfg, loop, snap.pred, snap.dom);
    if (!pre || *pre >= cfg.size()) {
        return;
    }
    std::vector<LatchEdit> latchEdits;
    for (const Group& group : groups) {
        const int derived = pinTemp(procedure, strings, group.shape);
        int stepOperand = group.factor;
        const Step& step = group.update.step;
        if (!step.immediate) {
            const int product = pinTemp(procedure, strings, group.shape);
            appendBeforeTerminator(cfg[*pre], ir::mul(step.value, group.factor, product));
            stepOperand = product;
        } else if (step.imm > 1) {
            const int constant = pinTemp(procedure, strings, group.shape);
            const int product = pinTemp(procedure, strings, group.shape);
            appendBeforeTerminator(cfg[*pre],
                    ir::assignConstant(strings.intern(std::to_string(step.imm)), constant));
            appendBeforeTerminator(cfg[*pre], ir::mul(constant, group.factor, product));
            stepOperand = product;
        }
        appendBeforeTerminator(cfg[*pre], ir::mul(group.iv, group.factor, derived));
        if (!step.immediate || step.imm != 0) {
            const Instruction bump = step.subtract
                    ? ir::sub(derived, stepOperand, derived)
                    : ir::add(derived, stepOperand, derived);
            latchEdits.push_back(LatchEdit { group.update.block, group.update.index, bump });
        }
        for (const MulSite& site : group.sites) {
            cfg[site.block].insts[site.index] = ir::assign(derived, site.result);
            ++stats.reduced;
        }
    }
    insertLatchEdits(cfg, std::move(latchEdits));
}

void rewriteAddresses(Cfg& cfg, Procedure& procedure, IrStringTable& strings, const LoopSnap& snap,
        const NaturalLoop& loop, const std::vector<AddressGroup>& groups, StrengthReduceStats& stats) {
    const auto pre = preheaderIndex(cfg, loop, snap.pred, snap.dom);
    if (!pre || *pre >= cfg.size()) {
        return;
    }
    std::vector<LatchEdit> latchEdits;
    for (const AddressGroup& group : groups) {
        const int derived = pinTemp(procedure, strings, group.shape);
        if (group.pointerForm) {
            appendBeforeTerminator(cfg[*pre], ir::pointerOffset(
                    group.base, group.iv, group.stride, derived, group.pointerSubtract));
        } else {
            appendBeforeTerminator(cfg[*pre], ir::indexAddress(
                    group.base, group.iv, group.stride, derived, group.baseMode));
        }
        if (group.byteDelta != 0) {
            latchEdits.push_back(LatchEdit { group.update.block, group.update.index,
                    ir::pointerAdd(derived, group.byteDelta, derived) });
        }
        for (const AddressSite& site : group.sites) {
            cfg[site.block].insts[site.index] = ir::assign(derived, site.result);
            ++stats.reduced;
        }
    }
    insertLatchEdits(cfg, std::move(latchEdits));
}

} // namespace

StrengthReduceStats strengthReduce(Procedure& procedure, IrStringTable& strings) {
    StrengthReduceStats stats;
    if (!hasBackwardJump(procedure.body)) {
        return stats;
    }
    Cfg cfg = buildCfg(procedure.body);
    LoopSnap snap = analyzeLoops(cfg);
    UseDefIndex index = buildUseDefIndex(cfg, procedure);

    std::unordered_set<int> need;
    for (const auto& loop : snap.loops) {
        if (loopNeedsPreheader(cfg, snap, index, loop)) {
            need.insert(cfg[loop.header].label);
        }
    }
    stats.inserted = insertPreheadersFor(cfg, strings, std::move(need));
    if (stats.inserted != 0) {
        snap = analyzeLoops(cfg);
        index = buildUseDefIndex(cfg, procedure);
    }

    std::vector<NaturalLoop> loops = snap.loops;
    std::sort(loops.begin(), loops.end(), [](const NaturalLoop& a, const NaturalLoop& b) {
        if (a.blocks.size() != b.blocks.size()) {
            return a.blocks.size() < b.blocks.size();
        }
        return a.header < b.header;
    });
    bool indexStale = false;
    for (const auto& loop : loops) {
        if (indexStale) {
            index = buildUseDefIndex(cfg, procedure);
            indexStale = false;
        }
        const std::vector<Group> groups = reducibleMuls(cfg, snap, index, loop);
        if (!groups.empty()) {
            rewriteGroups(cfg, procedure, strings, snap, loop, groups, stats);
            indexStale = true;
        }
        if (indexStale) {
            index = buildUseDefIndex(cfg, procedure);
            indexStale = false;
        }
        const std::vector<AddressGroup> addresses = reducibleAddresses(cfg, snap, index, loop);
        if (!addresses.empty()) {
            rewriteAddresses(cfg, procedure, strings, snap, loop, addresses, stats);
            indexStale = true;
        }
    }
    if (stats.reduced != 0 || stats.inserted != 0) {
        procedure.body = flattenCfg(cfg);
    }
    return stats;
}

} // namespace codegen
