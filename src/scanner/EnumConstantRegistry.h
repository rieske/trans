#ifndef ENUMCONSTANTREGISTRY_H_
#define ENUMCONSTANTREGISTRY_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "types/IntegerConstant.h"

namespace scanner {

struct ClosedEnumBody {
    std::vector<std::pair<std::string, type::IntegerConstant>> enumerators;
    bool hasRange { false };
    type::SignedBits min { 0 };
    type::SignedBits max { 0 };
};

// Enumerators are block-scoped. File-scope (root) stays until the session dies.
// add() is last-wins in the current frame; redefinition diagnostics belong at PE/SA.
// Open bodies are pushed by EnumBody `{` and popped by endEnumDefinition, not by `}`.
class EnumConstantRegistry {
public:
    void add(const std::string& name, type::IntegerConstant value);
    bool lookup(std::string_view name, type::IntegerConstant& value) const;
    std::optional<int> bindingDepth(std::string_view name) const;
    bool contains(std::string_view name) const;
    bool containsInCurrentScope(std::string_view name) const;

    void enterScope();
    void leaveScope();

    void openBody();
    std::optional<type::IntegerConstant> nextInCurrentBody() const;
    void recordInCurrentBody(std::string name, type::IntegerConstant value);
    ClosedEnumBody closeBody();

private:
    struct OpenEnumBody {
        std::vector<std::pair<std::string, type::IntegerConstant>> enumerators;
        std::optional<type::IntegerConstant> next;
        type::SignedBits min { 0 };
        type::SignedBits max { 0 };
        bool hasRange { false };
    };

    std::vector<std::map<std::string, type::IntegerConstant, std::less<>>> scopes_ { {} };
    std::vector<OpenEnumBody> bodies_;
};

} // namespace scanner

#endif // ENUMCONSTANTREGISTRY_H_
