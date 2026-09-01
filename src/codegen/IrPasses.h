#ifndef CODEGEN_IR_PASSES_H_
#define CODEGEN_IR_PASSES_H_

#include "Instruction.h"

#include <utility>

namespace codegen {

void sealProcedure(Procedure& procedure);
IntermediateRepresentation sealProcedures(IntermediateRepresentation ir);

void eliminateJumpToNext(std::vector<Instruction>& code);
IntermediateRepresentation eliminateJumpToNext(IntermediateRepresentation ir);

// Mid-end pass manager. packFrameValues stays after this list (IrGenerator.cpp).
using IrPass = IntermediateRepresentation (*)(IntermediateRepresentation);

inline constexpr IrPass kMidEndPasses[] = {
    sealProcedures,
    eliminateJumpToNext,
};

inline IntermediateRepresentation runIrPasses(IntermediateRepresentation ir) {
    for (auto pass : kMidEndPasses) {
        ir = pass(std::move(ir));
    }
    return ir;
}

} // namespace codegen

#endif // CODEGEN_IR_PASSES_H_
