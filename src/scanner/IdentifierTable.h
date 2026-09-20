#ifndef IDENTIFIERTABLE_H_
#define IDENTIFIERTABLE_H_

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "scanner/NameIntern.h"
#include "types/Type.h"

namespace scanner {

class IdentifierTable {
public:
    explicit IdentifierTable(NameIntern& intern);

    void addTypedef(const std::string& name, const type::Type& type);
    bool hasTypedef(std::string_view name) const;
    std::optional<type::Type> lookupTypedef(std::string_view name) const;

    void enterScope();
    void leaveScope();

    void addIdentifierShadow(const std::string& name);
    bool isIdentifierShadow(std::string_view name) const;

    void addPendingParameterShadow(const std::string& name);
    void clearPendingParameterShadows();
    unsigned revision() const { return revision_; }

private:
    struct Scope {
        std::unordered_map<int, type::Type> typedefs;
        std::unordered_set<int> shadows;
    };

    void flushPendingParameterShadows();

    NameIntern& intern_;
    unsigned revision_ { 0 };
    std::vector<Scope> scopes_ { Scope {} };
    std::unordered_set<int> pendingParameterShadows_;
};

} // namespace scanner

#endif
