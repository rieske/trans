#ifndef CODEGEN_LICM_H_
#define CODEGEN_LICM_H_

#include "Instruction.h"

namespace codegen {

struct LicmStats {
    int loopsVisited { 0 };
    int hoisted { 0 };
    int inserted { 0 };
};

bool isLicmPureOp(Op op);
LicmStats hoistLoopInvariants(Procedure& procedure, IrStringTable& strings);

} // namespace codegen

#endif // CODEGEN_LICM_H_
