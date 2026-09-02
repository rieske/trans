#ifndef CODEGEN_IR_PASSES_H_
#define CODEGEN_IR_PASSES_H_

#include "Instruction.h"

#include <utility>

namespace codegen {

void sealProcedure(Procedure& procedure);
IntermediateRepresentation sealProcedures(IntermediateRepresentation ir);

IntermediateRepresentation applyCfgPasses(IntermediateRepresentation ir, int optLevel = 1);

inline IntermediateRepresentation runIrPasses(IntermediateRepresentation ir, int optLevel = 1) {
    ir = sealProcedures(std::move(ir));
    return applyCfgPasses(std::move(ir), optLevel);
}

} // namespace codegen

#endif // CODEGEN_IR_PASSES_H_
