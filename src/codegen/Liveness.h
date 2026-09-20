#ifndef CODEGEN_LIVENESS_H_
#define CODEGEN_LIVENESS_H_

#include "Instruction.h"

#include <unordered_map>
#include <unordered_set>

namespace codegen {

struct ProcedureLiveness {
    std::unordered_map<int, std::unordered_set<int>> atLabel;
    std::unordered_map<int, std::unordered_set<int>> afterCall;
    std::vector<std::unordered_set<int>> afterInst;
    std::unordered_set<int> addressTaken;
};

struct LabelLiveIns {
    std::unordered_map<int, std::unordered_set<int>> atLabel;
};

ProcedureLiveness computeProcedureLiveness(const Procedure& procedure);
LabelLiveIns computeLabelLiveIns(const Procedure& procedure);
std::unordered_map<int, std::unordered_set<int>> computeLiveAfterCalls(const Procedure& procedure);

} // namespace codegen

#endif // CODEGEN_LIVENESS_H_
