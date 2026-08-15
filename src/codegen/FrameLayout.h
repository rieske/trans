#ifndef CODEGEN_FRAMELAYOUT_H_
#define CODEGEN_FRAMELAYOUT_H_

#include <vector>

#include "Instruction.h"
#include "Value.h"

namespace codegen {

// Named locals and address-taken temps keep distinct slots. Expression temps
// (including multi-word) reuse words after their last use.
std::vector<Value> packFrameValues(
        std::vector<Value> locals,
        const std::vector<Instruction>& body);

} // namespace codegen

#endif
