#ifndef CODEGEN_IR_STRING_TABLE_H_
#define CODEGEN_IR_STRING_TABLE_H_

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace codegen {

constexpr int kNoSymbol = -1;

class IrStringTable {
public:
    int intern(std::string_view text);
    int find(std::string_view text) const;
    int require(std::string_view text) const;
    const std::string& get(int id) const;
    int size() const { return static_cast<int>(names_.size()); }

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

    std::vector<std::string> names_;
    std::unordered_map<std::string, int, TransparentHash, std::equal_to<>> index_;
};

} // namespace codegen

#endif // CODEGEN_IR_STRING_TABLE_H_
