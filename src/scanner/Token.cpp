#include "Token.h"

#include <algorithm>
#include <vector>

namespace scanner {
namespace {

class TextArena {
public:
    std::string_view store(std::string_view text) {
        if (text.empty()) {
            return {};
        }
        if (chunks_.empty() || chunks_.back().size() + text.size() > chunks_.back().capacity()) {
            chunks_.emplace_back();
            chunks_.back().reserve(std::max(kChunk, text.size()));
        }
        auto& chunk = chunks_.back();
        const auto off = chunk.size();
        chunk.insert(chunk.end(), text.begin(), text.end());
        return { chunk.data() + off, text.size() };
    }

private:
    static constexpr std::size_t kChunk = 1 << 16;
    std::vector<std::vector<char>> chunks_;
};

TextArena& arena() {
    static TextArena pool;
    return pool;
}

std::string_view store(std::string_view text) {
    return arena().store(text);
}

} // namespace

const std::string Token::END = "'$end$'";

Token::Token(std::string_view id, std::string_view lexeme, const translation_unit::Context& context, int symbolId) :
        id { store(id.empty() ? std::string_view { END } : id) },
        lexeme { store(lexeme) },
        context { context },
        symbolId { symbolId }
{
}

} // namespace scanner

