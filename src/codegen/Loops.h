#ifndef CODEGEN_LOOPS_H_
#define CODEGEN_LOOPS_H_

#include "Cfg.h"

#include <cstddef>
#include <unordered_set>
#include <vector>

namespace codegen {

std::vector<std::vector<std::size_t>> cfgPredecessors(const Cfg& cfg);
std::vector<std::unordered_set<std::size_t>> dominators(const Cfg& cfg);

struct NaturalLoop {
    std::size_t header { 0 };
    std::unordered_set<std::size_t> blocks;
    std::vector<std::size_t> latches;
};

std::vector<NaturalLoop> naturalLoops(const Cfg& cfg);

} // namespace codegen

#endif // CODEGEN_LOOPS_H_
