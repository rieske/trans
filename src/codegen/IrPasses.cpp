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

IntermediateRepresentation applyCfgPasses(IntermediateRepresentation ir) {
    for (auto& procedure : ir.procedures) {
        procedure.body = flattenCfg(
                eliminateJumpToNext(eliminateUnreachable(buildCfg(procedure.body))));
    }
    return ir;
}

} // namespace codegen
