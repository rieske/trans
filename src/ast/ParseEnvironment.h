#ifndef AST_PARSEENVIRONMENT_H_
#define AST_PARSEENVIRONMENT_H_

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "scanner/LexicalSession.h"
#include "types/Type.h"

namespace ast {

// Parse-time symbol environment for one translation unit: struct/union tags
// and typedef/enum names (via LexicalSession). Separate from the bottom-up
// reduction stacks on AbstractSyntaxTreeBuilderContext.
class ParseEnvironment {
public:
    // Caller owns session; it must outlive this environment.
    explicit ParseEnvironment(scanner::LexicalSession& session);

    // Returns existing tag type or creates an incomplete record placeholder for the tag
    // (completed later as struct or union). Sole public tag API.
    type::Type ensureStructTag(const std::string& tag);

    void defineTypedef(const std::string& name, type::Type type);
    std::optional<type::Type> lookupTypedef(const std::string& name) const;

    void addEnumerator(std::string name, std::optional<long> explicitValue = std::nullopt);
    bool lookupEnumConstant(const std::string& name, long& value) const;
    std::vector<std::pair<std::string, long>> endEnumDefinition();
    // Snapshot of all parse-time enumerators registered on the session (for SA import).
    std::map<std::string, long> enumConstantsSnapshot() const;

private:
    void beginEnumDefinition();

    scanner::LexicalSession& session_;
    std::map<std::string, type::Type> structTags_;
    // Enum definition frames: each entry is (next_value, enumerators_so_far).
    // Nested enums inside an outer enumerator const_exp are not supported
    // (begin only when the stack is empty; see addEnumerator).
    std::vector<std::pair<long, std::vector<std::pair<std::string, long>>>> enumDefinitionStack_;
};

} // namespace ast

#endif // AST_PARSEENVIRONMENT_H_
