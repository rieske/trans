#ifndef CODEGEN_CFG_H_
#define CODEGEN_CFG_H_

#include "Instruction.h"

namespace codegen {

// Transient mid-end form. Procedure::body stays linear.
struct BasicBlock {
    int label { kNoSymbol };
    std::vector<Instruction> insts;
};

using Cfg = std::vector<BasicBlock>;

Cfg buildCfg(const std::vector<Instruction>& body);
std::vector<Instruction> flattenCfg(const Cfg& cfg);

// A block after an unconditional terminator must be labeled.
void validateCfg(const Cfg& cfg);
void validateProcedureBody(const std::vector<Instruction>& body);

} // namespace codegen

#endif // CODEGEN_CFG_H_
