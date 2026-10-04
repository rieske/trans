#ifndef CODEGEN_LIVENESS_H_
#define CODEGEN_LIVENESS_H_

#include "Instruction.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace codegen {

struct ProcedureLiveness {
    std::unordered_map<int, std::unordered_set<int>> atLabel;
    std::unordered_map<int, std::unordered_set<int>> afterCall;
    std::unordered_set<int> addressTaken;
};

struct TempLiveness {
    std::vector<char> resultLiveAfter;
    std::unordered_set<int> addressTaken;
};

ProcedureLiveness computeProcedureLiveness(const Procedure& procedure);
TempLiveness computeTempLiveness(const Procedure& procedure);

} // namespace codegen

#endif // CODEGEN_LIVENESS_H_
