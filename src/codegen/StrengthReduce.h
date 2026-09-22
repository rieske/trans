#ifndef CODEGEN_STRENGTH_REDUCE_H_
#define CODEGEN_STRENGTH_REDUCE_H_

#include "Instruction.h"

namespace codegen {

struct StrengthReduceStats {
    int reduced { 0 };
    int inserted { 0 };
};

StrengthReduceStats strengthReduce(Procedure& procedure, IrStringTable& strings);

} // namespace codegen

#endif // CODEGEN_STRENGTH_REDUCE_H_
