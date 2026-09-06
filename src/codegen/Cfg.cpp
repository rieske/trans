#include "Cfg.h"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace codegen {

namespace {

bool endsWithUnconditionalTerminator(const BasicBlock& block) {
    if (block.insts.empty()) {
        return false;
    }
    const Instruction& last = block.insts.back();
    return instructionTransfersControl(last)
            && (last.op != Op::Jump || last.cond == JumpCondition::UNCONDITIONAL);
}

std::size_t blockIndexWithLabel(const Cfg& cfg, int label) {
    for (std::size_t i = 0; i < cfg.size(); ++i) {
        if (cfg[i].label == label) {
            return i;
        }
    }
    return cfg.size();
}

std::vector<std::size_t> successors(const Cfg& cfg, std::size_t i) {
    std::vector<std::size_t> succs;
    auto addFallthrough = [&]() {
        if (i + 1 < cfg.size()) {
            succs.push_back(i + 1);
        }
    };
    const BasicBlock& block = cfg[i];
    if (block.insts.empty() || !instructionTransfersControl(block.insts.back())) {
        addFallthrough();
        return succs;
    }
    const Instruction& last = block.insts.back();
    if (last.op == Op::Jump) {
        const std::size_t target = blockIndexWithLabel(cfg, last.arg0);
        if (target < cfg.size()) {
            succs.push_back(target);
        }
        if (last.cond != JumpCondition::UNCONDITIONAL) {
            addFallthrough();
        }
        return succs;
    }
    return succs;
}

} // namespace

Cfg buildCfg(const std::vector<Instruction>& body) {
    Cfg cfg;
    BasicBlock current;

    auto flush = [&]() {
        if (current.label != kNoSymbol || !current.insts.empty()) {
            cfg.push_back(std::move(current));
        }
        current = BasicBlock {};
    };

    for (const auto& instruction : body) {
        if (instruction.op == Op::Label) {
            flush();
            current.label = instruction.arg0;
            continue;
        }
        current.insts.push_back(instruction);
        if (instructionTransfersControl(instruction)) {
            flush();
        }
    }
    flush();
    return cfg;
}

std::vector<Instruction> flattenCfg(const Cfg& cfg) {
    std::vector<Instruction> body;
    for (const auto& block : cfg) {
        if (block.label != kNoSymbol) {
            body.push_back(ir::label(block.label));
        }
        body.insert(body.end(), block.insts.begin(), block.insts.end());
    }
    return body;
}

Cfg threadJumps(Cfg cfg) {
    std::unordered_map<int, int> trampoline;
    for (const auto& block : cfg) {
        if (block.label == kNoSymbol || block.insts.size() != 1) {
            continue;
        }
        const Instruction& inst = block.insts.front();
        if (inst.op == Op::Jump && inst.cond == JumpCondition::UNCONDITIONAL
                && inst.arg0 != kNoSymbol) {
            trampoline[block.label] = inst.arg0;
        }
    }
    if (trampoline.empty()) {
        return cfg;
    }

    auto finalTarget = [&](int label) {
        std::unordered_set<int> seen;
        int target = label;
        while (true) {
            const auto it = trampoline.find(target);
            if (it == trampoline.end()) {
                return target;
            }
            if (!seen.insert(target).second) {
                return label;
            }
            target = it->second;
        }
    };

    for (auto& block : cfg) {
        for (auto& inst : block.insts) {
            if (inst.op != Op::Jump) {
                continue;
            }
            const int dest = finalTarget(inst.arg0);
            if (dest != inst.arg0) {
                inst.arg0 = dest;
            }
        }
    }
    return cfg;
}

Cfg eliminateUnreachable(Cfg cfg) {
    if (cfg.empty()) {
        return cfg;
    }
    std::vector<char> reachable(cfg.size(), 0);
    std::vector<std::size_t> work { 0 };
    reachable[0] = 1;
    while (!work.empty()) {
        const std::size_t i = work.back();
        work.pop_back();
        for (const std::size_t s : successors(cfg, i)) {
            if (!reachable[s]) {
                reachable[s] = 1;
                work.push_back(s);
            }
        }
    }
    Cfg out;
    out.reserve(cfg.size());
    for (std::size_t i = 0; i < cfg.size(); ++i) {
        if (reachable[i]) {
            out.push_back(std::move(cfg[i]));
        }
    }
    return out;
}

Cfg eliminateJumpToNext(Cfg cfg) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = 0; i + 1 < cfg.size(); ++i) {
            if (cfg[i].insts.empty() || cfg[i + 1].label == kNoSymbol) {
                continue;
            }
            const Instruction& last = cfg[i].insts.back();
            if (last.op == Op::Jump && last.cond == JumpCondition::UNCONDITIONAL
                    && last.arg0 == cfg[i + 1].label) {
                cfg[i].insts.pop_back();
                changed = true;
            }
        }
    }
    return cfg;
}

void validateCfg(const Cfg& cfg) {
    for (std::size_t i = 1; i < cfg.size(); ++i) {
        if (endsWithUnconditionalTerminator(cfg[i - 1]) && cfg[i].label == kNoSymbol) {
            throw std::logic_error { "procedure body is not implicit blocks" };
        }
    }
}

void validateProcedureBody(const std::vector<Instruction>& body) {
    validateCfg(buildCfg(body));
}

} // namespace codegen
