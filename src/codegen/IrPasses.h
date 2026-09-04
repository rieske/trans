#ifndef CODEGEN_IR_PASSES_H_
#define CODEGEN_IR_PASSES_H_

#include "Instruction.h"

namespace codegen {

void sealProcedure(Procedure& procedure);
IntermediateRepresentation sealProcedures(IntermediateRepresentation ir);

void foldConstants(Procedure& procedure, IrStringTable& strings);
void eliminateDeadTemps(Procedure& procedure);

IntermediateRepresentation applyCfgPasses(IntermediateRepresentation ir, int optLevel = 1);

IntermediateRepresentation runIrPasses(IntermediateRepresentation ir, int optLevel = 1);

} // namespace codegen

#endif // CODEGEN_IR_PASSES_H_
