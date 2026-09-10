#ifndef CODEGEN_LIVENESS_H_
#define CODEGEN_LIVENESS_H_

#include "Instruction.h"

#include <unordered_map>
#include <unordered_set>

namespace codegen {

struct LabelLiveIns {
    std::unordered_map<int, std::unordered_set<int>> atLabel;
};

LabelLiveIns computeLabelLiveIns(const Procedure& procedure);

} // namespace codegen

#endif // CODEGEN_LIVENESS_H_
