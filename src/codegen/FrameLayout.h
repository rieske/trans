#ifndef CODEGEN_FRAMELAYOUT_H_
#define CODEGEN_FRAMELAYOUT_H_

#include <vector>

#include "Instruction.h"
#include "Value.h"

namespace codegen {

// Named locals, address-taken temps, and multi-word temps keep distinct slots.
// One-word expression temps reuse a slot after their last use.
std::vector<Value> packFrameValues(
        std::vector<Value> locals,
        const std::vector<Instruction>& body);

} // namespace codegen

#endif
