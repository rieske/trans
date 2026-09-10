#ifndef CODEGEN_CFG_H_
#define CODEGEN_CFG_H_

#include "Instruction.h"

#include <cstddef>

namespace codegen {

// Transient mid-end form. Procedure::body stays linear.
struct BasicBlock {
    int label { kNoSymbol };
    std::vector<Instruction> insts;
};

using Cfg = std::vector<BasicBlock>;

Cfg buildCfg(const std::vector<Instruction>& body);
std::vector<std::size_t> cfgSuccessors(const Cfg& cfg, std::size_t blockIndex);
std::vector<Instruction> flattenCfg(const Cfg& cfg);
Cfg threadJumps(Cfg cfg);
Cfg eliminateUnreachable(Cfg cfg);
Cfg eliminateJumpToNext(Cfg cfg);

// A block after an unconditional terminator must be labeled.
void validateCfg(const Cfg& cfg);
void validateProcedureBody(const std::vector<Instruction>& body);

} // namespace codegen

#endif // CODEGEN_CFG_H_
