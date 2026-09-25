#ifndef CODEGEN_IR_PASSES_H_
#define CODEGEN_IR_PASSES_H_

#include "Instruction.h"

namespace codegen {

void sealProcedure(Procedure& procedure);
IntermediateRepresentation sealProcedures(IntermediateRepresentation ir);

struct FoldResult {
    bool changed { false };
    bool controlFlow { false };
};

FoldResult foldConstants(Procedure& procedure, IrStringTable& strings);
void copyPropagate(Procedure& procedure);
void forwardLocalLoads(Procedure& procedure);
void eliminateDeadTemps(Procedure& procedure);

void applyCfgPasses(Procedure& procedure, int optLevel);
IntermediateRepresentation applyCfgPasses(IntermediateRepresentation ir, int optLevel = 1);

IntermediateRepresentation runIrPasses(IntermediateRepresentation ir, int optLevel = 1);

} // namespace codegen

#endif // CODEGEN_IR_PASSES_H_
