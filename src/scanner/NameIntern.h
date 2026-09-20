#ifndef NAMEINTERN_H_
#define NAMEINTERN_H_

#include <string>
#include <string_view>
#include <unordered_map>

namespace scanner {

constexpr int kNoName = -1;

class NameIntern {
public:
    int intern(std::string_view text);
    int find(std::string_view text) const;
    int size() const { return static_cast<int>(index_.size()); }

private:
    struct TransparentHash {
        using is_transparent = void;
        std::size_t operator()(std::string_view text) const noexcept {
            return std::hash<std::string_view> {}(text);
        }
        std::size_t operator()(const std::string& text) const noexcept {
            return std::hash<std::string_view> {}(text);
        }
    };

    std::unordered_map<std::string, int, TransparentHash, std::equal_to<>> index_;
};

} // namespace scanner

#endif
