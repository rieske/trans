#include "IrPasses.h"

#include <utility>

namespace codegen {

void sealProcedure(Procedure& procedure) {
    if (procedure.body.empty() || !instructionTransfersControl(procedure.body.back())) {
        procedure.body.push_back(ir::voidReturn());
    }
}

IntermediateRepresentation sealProcedures(IntermediateRepresentation ir) {
    for (auto& procedure : ir.procedures) {
        sealProcedure(procedure);
    }
    return ir;
}

void eliminateJumpToNext(std::vector<Instruction>& code) {
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<Instruction> out;
        out.reserve(code.size());
        for (std::size_t i = 0; i < code.size(); ++i) {
            if (i + 1 < code.size()
                    && code[i].op == Op::Jump
                    && code[i].cond == JumpCondition::UNCONDITIONAL
                    && code[i + 1].op == Op::Label
                    && code[i].arg0 == code[i + 1].arg0) {
                changed = true;
                continue;
            }
            out.push_back(std::move(code[i]));
        }
        code = std::move(out);
    }
}

IntermediateRepresentation eliminateJumpToNext(IntermediateRepresentation ir) {
    for (auto& procedure : ir.procedures) {
        eliminateJumpToNext(procedure.body);
    }
    return ir;
}

} // namespace codegen
