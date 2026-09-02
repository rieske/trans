#include "IrPasses.h"

#include "Cfg.h"

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

IntermediateRepresentation applyCfgPasses(IntermediateRepresentation ir, int optLevel) {
    for (auto& procedure : ir.procedures) {
        Cfg cfg = buildCfg(procedure.body);
        if (optLevel >= 1) {
            cfg = eliminateUnreachable(std::move(cfg));
        }
        procedure.body = flattenCfg(eliminateJumpToNext(std::move(cfg)));
    }
    return ir;
}

} // namespace codegen
