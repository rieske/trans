#ifndef GENERATION_BUFFERS_H_
#define GENERATION_BUFFERS_H_

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace parser {

// Reusable scratch space for one parsing-table generation.
// Owned by CanonicalCollection; shared by closure and LALR construction.
struct GenerationBuffers {
    std::unordered_map<std::uint64_t, std::size_t> coreIndex;
    std::vector<std::size_t> worklist;
    std::vector<std::uint64_t> coreSignature;

    void prepareClosure(std::size_t itemHint) {
        coreIndex.clear();
        worklist.clear();
        if (coreIndex.bucket_count() < 64) {
            coreIndex.reserve(128);
        }
        worklist.reserve(std::max(worklist.capacity(), itemHint * 4 + 16));
    }
};

} // namespace parser

#endif
