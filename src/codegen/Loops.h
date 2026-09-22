#ifndef CODEGEN_LOOPS_H_
#define CODEGEN_LOOPS_H_

#include "Cfg.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>

namespace codegen {

struct DomBits {
    std::vector<std::uint64_t> words;
    bool test(std::size_t i) const {
        const std::size_t w = i / 64;
        return w < words.size() && (words[w] & (std::uint64_t { 1 } << (i % 64))) != 0;
    }
    bool none() const {
        for (std::uint64_t w : words) {
            if (w != 0) {
                return false;
            }
        }
        return true;
    }
};

bool hasBackwardJump(const std::vector<Instruction>& body);

std::vector<std::vector<std::size_t>> cfgPredecessors(const Cfg& cfg);
std::vector<DomBits> dominators(const Cfg& cfg);
std::vector<DomBits> dominators(const Cfg& cfg, const std::vector<std::vector<std::size_t>>& pred);

struct NaturalLoop {
    std::size_t header { 0 };
    std::unordered_set<std::size_t> blocks;
    std::vector<std::size_t> latches;
};

std::vector<NaturalLoop> naturalLoops(const Cfg& cfg);
std::vector<NaturalLoop> naturalLoops(const Cfg& cfg,
        const std::vector<std::vector<std::size_t>>& pred, const std::vector<DomBits>& dom);

bool hasPreheader(const Cfg& cfg, const NaturalLoop& loop);
bool hasPreheader(const Cfg& cfg, const NaturalLoop& loop,
        const std::vector<std::vector<std::size_t>>& pred, const std::vector<DomBits>& dom);
std::optional<std::size_t> preheaderIndex(const Cfg& cfg, const NaturalLoop& loop);
std::optional<std::size_t> preheaderIndex(const Cfg& cfg, const NaturalLoop& loop,
        const std::vector<std::vector<std::size_t>>& pred, const std::vector<DomBits>& dom);

} // namespace codegen

#endif // CODEGEN_LOOPS_H_
