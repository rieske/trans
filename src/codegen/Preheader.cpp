#include "Preheader.h"

#include "Instruction.h"
#include "InternalError.h"
#include "IrBuilders.h"

#include <algorithm>
#include <cstddef>
#include <unordered_set>

namespace codegen {
namespace {

bool isUnconditionalTerminator(const Instruction& inst) {
    if (inst.op == Op::Return || inst.op == Op::VoidReturn) {
        return true;
    }
    return inst.op == Op::Jump && inst.cond == JumpCondition::UNCONDITIONAL;
}

bool endsWithUncondTerminator(const BasicBlock& block) {
    return !block.insts.empty() && isUnconditionalTerminator(block.insts.back());
}

bool endsWithTransfer(const BasicBlock& block) {
    return !block.insts.empty() && instructionTransfersControl(block.insts.back());
}

NaturalLoop loopWithHeaderLabel(const Cfg& cfg, int headerLabel) {
    for (auto& loop : naturalLoops(cfg)) {
        if (loop.header < cfg.size() && cfg[loop.header].label == headerLabel) {
            return loop;
        }
    }
    internalError("insertOne: header disappeared");
}

bool fallsThroughToHeader(const Cfg& cfg, const NaturalLoop& loop) {
    const std::size_t h = loop.header;
    if (h == 0) {
        return false;
    }
    const std::size_t pred = h - 1;
    bool isLatch = false;
    for (const std::size_t latch : loop.latches) {
        if (latch == pred) {
            isLatch = true;
            break;
        }
    }
    if (!isLatch) {
        return false;
    }
    return !endsWithUncondTerminator(cfg[pred]);
}

} // namespace

void insertOne(Cfg& cfg, const NaturalLoop& loopIn, IrStringTable& strings) {
    NaturalLoop loop = loopIn;
    if (loop.header >= cfg.size()) {
        return;
    }
    std::size_t h = loop.header;
    const int headerLabel = cfg[h].label;
    if (h != 0 && headerLabel == kNoSymbol) {
        return;
    }

    if (fallsThroughToHeader(cfg, loop)) {
        BasicBlock& latch = cfg[h - 1];
        if (!endsWithTransfer(latch)) {
            latch.insts.push_back(ir::jump(headerLabel));
        } else {
            BasicBlock extra;
            extra.label = strings.internFresh("__L");
            extra.insts.push_back(ir::jump(headerLabel));
            cfg.insert(cfg.begin() + static_cast<std::ptrdiff_t>(h), std::move(extra));
            loop = loopWithHeaderLabel(cfg, headerLabel);
            h = loop.header;
        }
    }

    const int preLabel = strings.internFresh("__L");
    std::unordered_set<std::size_t> latchSet(loop.latches.begin(), loop.latches.end());
    std::vector<std::size_t> retarget;
    for (std::size_t i = 0; i < cfg.size(); ++i) {
        if (latchSet.count(i) != 0) {
            continue;
        }
        for (const auto& inst : cfg[i].insts) {
            if (inst.op == Op::Jump && inst.arg0 == headerLabel) {
                retarget.push_back(i);
                break;
            }
        }
    }

    BasicBlock pre;
    pre.label = preLabel;
    cfg.insert(cfg.begin() + static_cast<std::ptrdiff_t>(h), std::move(pre));
    for (const std::size_t p : retarget) {
        const std::size_t np = p < h ? p : p + 1;
        for (auto& inst : cfg[np].insts) {
            if (inst.op == Op::Jump && inst.arg0 == headerLabel) {
                inst.arg0 = preLabel;
            }
        }
    }
}

PreheaderStats insertPreheaders(Cfg& cfg, IrStringTable& strings) {
    PreheaderStats stats;
    for (;;) {
        const auto pred = cfgPredecessors(cfg);
        const auto dom = dominators(cfg, pred);
        auto loops = naturalLoops(cfg, pred, dom);
        std::sort(loops.begin(), loops.end(),
                [](const NaturalLoop& a, const NaturalLoop& b) { return a.header > b.header; });
        bool did = false;
        for (const auto& loop : loops) {
            if (loop.header >= cfg.size()) {
                continue;
            }
            if (hasPreheader(cfg, loop, pred, dom)) {
                continue;
            }
            if (cfg[loop.header].label == kNoSymbol && loop.header != 0) {
                continue;
            }
            insertOne(cfg, loop, strings);
            ++stats.inserted;
            did = true;
            break;
        }
        if (!did) {
            break;
        }
    }
    return stats;
}

} // namespace codegen
