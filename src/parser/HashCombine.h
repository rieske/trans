#ifndef PARSER_HASH_COMBINE_H_
#define PARSER_HASH_COMBINE_H_

#include <cstddef>
#include <functional>
#include <utility>

namespace parser {

// Mix `value`'s hash into `seed` for multi-field / multi-element keys used with
// std::unordered_map (e.g. (state, symbol) pairs, LALR core signatures).
//
// This is the Boost hash_combine recipe (64-bit form):
//   seed ^= hash(v) + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
//
// The constant is floor(2^64 / φ) (golden-ratio related) so successive mixes
// scramble bits instead of clustering. The shifts spread earlier seed bits.
// Not cryptographic — only for bucket placement; equality still uses operator==.
//
// See also: boost::hash_combine
template<typename T>
inline void hashCombine(std::size_t& seed, const T& value) noexcept {
    seed ^= std::hash<T>{}(value) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

// Shared key for flat (state, symbol) maps: gotos, actions, computed transitions.
using StateSymbolKey = std::pair<std::size_t, int>;

struct StateSymbolHash {
    std::size_t operator()(const StateSymbolKey& value) const noexcept {
        std::size_t hash = std::hash<std::size_t>{}(value.first);
        hashCombine(hash, value.second);
        return hash;
    }
};

} // namespace parser

#endif
