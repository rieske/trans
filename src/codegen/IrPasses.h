#ifndef CODEGEN_IR_PASSES_H_
#define CODEGEN_IR_PASSES_H_

#include "Instruction.h"

namespace codegen {

void sealProcedure(Procedure& procedure);
IntermediateRepresentation sealProcedures(IntermediateRepresentation ir);

void eliminateJumpToNext(std::vector<Instruction>& code);
IntermediateRepresentation eliminateJumpToNext(IntermediateRepresentation ir);

IntermediateRepresentation runIrPasses(IntermediateRepresentation ir);

} // namespace codegen

#endif // CODEGEN_IR_PASSES_H_
