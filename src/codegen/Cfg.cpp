#include "Cfg.h"

#include <stdexcept>

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
