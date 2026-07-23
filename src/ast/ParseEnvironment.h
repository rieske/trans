#ifndef AST_PARSEENVIRONMENT_H_
#define AST_PARSEENVIRONMENT_H_

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "PendingArrayMemberStore.h"
#include "types/Type.h"

namespace ast {

// Parse-time symbol environment for one translation unit: struct/union tags,
// typedef names, enum enumerator accumulation, and deferred ARRAY_SIZE member
// bounds. Separate from the bottom-up reduction stacks on
// AbstractSyntaxTreeBuilderContext.
class ParseEnvironment {
public:
    // Returns existing tag type or creates an incomplete struct type for the tag.
    type::Type ensureStructTag(const std::string& tag);

    void defineTypedef(const std::string& name, type::Type type);
    std::optional<type::Type> lookupTypedef(const std::string& name) const;

    void addEnumerator(std::string name, std::optional<long> explicitValue = std::nullopt);
    std::vector<std::pair<std::string, long>> endEnumDefinition();

    PendingArrayMemberStore& pendingArrayMembers() { return pendingArrayMembers_; }
    const PendingArrayMemberStore& pendingArrayMembers() const { return pendingArrayMembers_; }
    PendingArrayMemberStore takePendingArrayMembers() { return std::move(pendingArrayMembers_); }

private:
    void beginEnumDefinition();

    std::map<std::string, type::Type> structTags_;
    std::map<std::string, type::Type> typedefs_;
    // Nested enum definitions: each entry is (next_value, enumerators_so_far).
    std::vector<std::pair<long, std::vector<std::pair<std::string, long>>>> enumDefinitionStack_;
    PendingArrayMemberStore pendingArrayMembers_;
};

} // namespace ast

#endif // AST_PARSEENVIRONMENT_H_
